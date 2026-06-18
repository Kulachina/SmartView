'use strict';
// Cloud Function для Яндекс Облака: активация и проверка лицензий SmartView.
// Хранилище — JSON-файл state.json в Object Storage (бакет S3_BUCKET).
// Подпись токена — Ed25519 приватным ключом (в base64 в переменной ED25519_PRIVATE_B64).

const crypto = require('crypto');
const { S3Client, GetObjectCommand, PutObjectCommand } = require('@aws-sdk/client-s3');

const BUCKET = process.env.S3_BUCKET;
const STATE_KEY = 'state.json';
const TOKEN_DAYS = parseInt(process.env.TOKEN_DAYS || '30', 10);

const s3 = new S3Client({
    region: 'ru-central1',
    endpoint: 'https://storage.yandexcloud.net',
    credentials: { accessKeyId: process.env.S3_KEY_ID, secretAccessKey: process.env.S3_SECRET },
});

let _priv = null;
function getPriv() {
    if (!_priv) {
        const pem = Buffer.from(process.env.ED25519_PRIVATE_B64 || '', 'base64').toString('utf8');
        _priv = crypto.createPrivateKey(pem);   // бросит понятную ошибку, если ключ кривой
    }
    return _priv;
}
const b64u = (b) => Buffer.from(b).toString('base64url');
const reply = (code, obj) => ({ statusCode: code, headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(obj) });

async function readState() {
    const out = await s3.send(new GetObjectCommand({ Bucket: BUCKET, Key: STATE_KEY }));
    return JSON.parse(await out.Body.transformToString());
}
async function writeState(state) {
    await s3.send(new PutObjectCommand({
        Bucket: BUCKET, Key: STATE_KEY,
        Body: JSON.stringify(state, null, 2), ContentType: 'application/json',
    }));
}
function signToken(key, fp) {
    const iat = Math.floor(Date.now() / 1000);
    const exp = iat + TOKEN_DAYS * 86400;
    const payload = JSON.stringify({ key_id: key, fp, iat, exp });
    const sig = crypto.sign(null, Buffer.from(payload), getPriv());   // Ed25519
    return { token: b64u(payload) + '.' + b64u(sig), expires: new Date(exp * 1000).toISOString() };
}

exports.handler = async (event) => {
  try {
    const path = (event.requestContext && event.requestContext.http && event.requestContext.http.path) || event.path || '';
    let body = {};
    try { body = JSON.parse(event.body || '{}'); } catch (e) { return reply(400, { error: 'bad json' }); }

    if (path.endsWith('/activate')) {
        const key = (body.key || '').trim();
        const fp = body.fingerprint || '';
        if (!key || !fp) return reply(400, { error: 'key/fingerprint required' });
        const state = await readState();
        const lic = (state.licenses || {})[key];
        if (!lic || lic.status !== 'active') return reply(403, { error: 'Ключ не найден или отозван' });
        if (lic.expires && new Date(lic.expires) < new Date()) return reply(403, { error: 'Срок ключа истёк' });
        state.devices = state.devices || {};
        const list = state.devices[key] || [];
        const known = list.includes(fp);
        if (!known && list.length >= (lic.max_devices || 1)) return reply(403, { error: 'Превышен лимит устройств' });
        if (!known) { list.push(fp); state.devices[key] = list; await writeState(state); }
        return reply(200, Object.assign(signToken(key, fp), { owner: lic.owner || '' }));
    }

    if (path.endsWith('/validate')) {
        const token = body.token || '';
        const fp = body.fingerprint || '';
        const [p, s] = token.split('.');
        if (!p || !s) return reply(200, { valid: false, reason: 'no token' });
        const payloadBuf = Buffer.from(p, 'base64url');
        const sigBuf = Buffer.from(s, 'base64url');
        // Проверка подписи Ed25519 публичным ключом (выводится из приватного).
        const pub = crypto.createPublicKey(getPriv());
        if (!crypto.verify(null, payloadBuf, pub, sigBuf)) {
            return reply(200, { valid: false, reason: 'bad signature' });
        }
        let payload;
        try { payload = JSON.parse(payloadBuf.toString()); }
        catch (e) { return reply(200, { valid: false, reason: 'bad payload' }); }
        if (payload.fp !== fp) return reply(200, { valid: false, reason: 'fp mismatch' });
        if (payload.exp && payload.exp * 1000 < Date.now()) return reply(200, { valid: false, reason: 'expired' });
        const state = await readState();
        const lic = (state.licenses || {})[payload.key_id];
        const list = (state.devices || {})[payload.key_id] || [];
        const ok = lic && lic.status === 'active' && list.includes(fp);
        return reply(200, { valid: !!ok, reason: ok ? '' : 'revoked or unbound' });
    }

    return reply(404, { error: 'unknown path' });
  } catch (e) {
    return reply(500, { error: 'server error: ' + (e && e.message ? e.message : String(e)) });
  }
};
