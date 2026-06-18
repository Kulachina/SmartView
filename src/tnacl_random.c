/* Заглушка randombytes для TweetNaCl.
 * При проверке подписи (crypto_sign_open) генерация случайных чисел не нужна,
 * но символ randombytes требуется линковщику, т.к. на него ссылается tweetnacl.c.
 * Если когда-нибудь понадобятся функции, использующие случайность (keypair/box),
 * сюда нужно подставить криптостойкий источник. */
void randombytes(unsigned char *x, unsigned long long xlen) {
    (void)x;
    (void)xlen;
}
