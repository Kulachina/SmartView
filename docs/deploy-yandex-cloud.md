# Деплой сервера лицензий SmartView в Яндекс Облако (пошагово)

Цель: поднять HTTPS-эндпоинт активации (`/activate`, `/validate`) на serverless-сервисах
Яндекс Облака. Хранилище лицензий — JSON-файл в Object Storage. Подпись токенов — Ed25519.

Схема: **Клиент → API Gateway (HTTPS) → Cloud Function → Object Storage (state.json)**.

Всё, что в `server/`, — готовые файлы для загрузки. Ничего администрировать постоянно не нужно.

---

## 0. Что понадобится
- Аккаунт в Яндекс Облаке + привязанный платёжный аккаунт (на старте дают грант).
- Установленный `yc` (CLI Яндекс Облака) и `openssl` (для генерации ключей).
- Понятия: **cloud** (облако) и **folder** (папка) — рабочее пространство; обычно уже есть `default`.

> Почти всё ниже можно сделать и мышкой в консоли (console.cloud.yandex.ru), и через `yc`.
> Команды `yc` дают точный воспроизводимый результат — рекомендую их.

---

## 1. Установить и настроить CLI
```bash
# установка (Windows: см. PowerShell-инструкцию на странице yc; или через WSL)
yc init        # откроет браузер для входа, выберите cloud и folder
yc config list # проверка: должны быть cloud-id и folder-id
```

---

## 2. Сгенерировать ключи Ed25519 (локально, один раз)
```bash
openssl genpkey -algorithm ed25519 -out private.pem   # СЕКРЕТ — никому не отдавать
openssl pkey -in private.pem -pubout -out public.pem  # публичный — пойдёт в клиент (Фаза 2)
```
Приватный ключ функции отдаём **в base64** (чтобы переносы строк PEM не ломали переменную):
```bash
# Linux/macOS:
base64 -w0 private.pem > private.b64
# Windows PowerShell:
[Convert]::ToBase64String([IO.File]::ReadAllBytes("private.pem")) | Out-File private.b64 -NoNewline
```
Содержимое `private.b64` пойдёт в переменную окружения `ED25519_PRIVATE_B64`.

---

## 3. Object Storage: бакет и файл лицензий
1. Создать бакет (имя глобально уникально):
   ```bash
   yc storage bucket create --name smartview-licenses
   ```
   (или в консоли: **Object Storage → Создать бакет**, доступ — приватный).
2. Подготовить `state.json` на основе `server/state.example.json`: впишите свой первый ключ.
3. Загрузить файл в бакет:
   ```bash
   yc storage s3api put-object --bucket smartview-licenses --key state.json --body state.json
   ```
   (или в консоли перетащить файл в бакет).

---

## 4. Сервисный аккаунт и доступ к Object Storage
Функция будет читать/писать `state.json` от имени сервис-аккаунта.
```bash
# создать сервис-аккаунт
yc iam service-account create --name smartview-fn

# дать ему доступ к бакету (роль storage.editor на папку)
FOLDER=$(yc config get folder-id)
SA_ID=$(yc iam service-account get --name smartview-fn --format json | jq -r .id)
yc resource-manager folder add-access-binding $FOLDER --role storage.editor --subject serviceAccount:$SA_ID

# статический ключ доступа (S3-совместимый) — для обращения к Object Storage из кода
yc iam access-key create --service-account-name smartview-fn
# ВЫВЕДЕТ: key_id (access_key.key_id) и secret (secret). СОХРАНИТЕ ОБА — secret больше не показать.
```
`key_id` → переменная `S3_KEY_ID`, `secret` → `S3_SECRET`.

---

## 5. Cloud Function
1. Собрать архив с кодом:
   ```bash
   cd server
   npm install            # подтянет @aws-sdk/client-s3 в node_modules
   zip -r ../function.zip index.js package.json node_modules
   cd ..
   ```
   (Windows без zip: «Отправить → Сжатая ZIP-папка» по содержимому `server`.)
2. Создать функцию:
   ```bash
   yc serverless function create --name smartview-license
   ```
3. Залить версию с настройками (runtime, точка входа, сервис-аккаунт, переменные):
   ```bash
   yc serverless function version create \
     --function-name smartview-license \
     --runtime nodejs18 \
     --entrypoint index.handler \
     --memory 128m --execution-timeout 10s \
     --service-account-id $SA_ID \
     --source-path ./function.zip \
     --environment S3_BUCKET=smartview-licenses \
     --environment S3_KEY_ID=<key_id_из_шага_4> \
     --environment S3_SECRET=<secret_из_шага_4> \
     --environment TOKEN_DAYS=30 \
     --environment ED25519_PRIVATE_B64=<содержимое_private.b64>
   ```
   (или всё это в консоли: **Cloud Functions → создать → редактор/ZIP → Параметры: окружение,
   сервис-аккаунт**.)

> Записать `FUNCTION_ID`: `yc serverless function get --name smartview-license --format json | jq -r .id`

---

## 6. API Gateway (даёт HTTPS-URL)
1. В `server/api-gateway.yaml` заменить:
   - `function_id` → ваш `FUNCTION_ID`;
   - `service_account_id` → сервис-аккаунт с ролью `functions.functionInvoker`.
     Можно тот же `smartview-fn`, добавив роль:
     ```bash
     yc resource-manager folder add-access-binding $FOLDER \
       --role functions.functionInvoker --subject serviceAccount:$SA_ID
     ```
2. Создать шлюз:
   ```bash
   yc serverless api-gateway create --name smartview-gw --spec server/api-gateway.yaml
   ```
3. Узнать домен:
   ```bash
   yc serverless api-gateway get --name smartview-gw --format json | jq -r .domain
   # пример: d5xxxxxxxx.apigw.yandexcloud.net
   ```
   Базовый URL = `https://<domain>` — это `kServerUrl` в клиенте.

---

## 7. Проверка
```bash
curl -X POST https://<domain>/activate \
  -H "Content-Type: application/json" \
  -d '{"key":"TEST-0001-AAAA-BBBB","fingerprint":"тест123","version":"1.0"}'
# Ожидаем: {"token":"...","expires":"...","owner":"..."}
```
Если вернулся `token` — сервер работает.

---

## 8. Включить лицензирование в приложении
В `src/licensemanager.h`:
```cpp
inline const QString kServerUrl = QStringLiteral("https://<domain>");
inline constexpr bool kEnforce = true;
```
Пересобрать. Теперь при первом запуске появится диалог активации.

---

## 9. Выдача и отзыв ключей (повседневное)
- **Выдать ключ:** скачать `state.json`, добавить запись в `licenses`, загрузить обратно:
  ```json
  "WXYZ-2025-1234-5678": { "owner": "Пётр", "status": "active", "max_devices": 1, "expires": "" }
  ```
  ```bash
  yc storage s3api get-object --bucket smartview-licenses --key state.json --file state.json   # скачать
  # отредактировать
  yc storage s3api put-object --bucket smartview-licenses --key state.json --body state.json   # залить
  ```
- **Отозвать:** поставить `"status": "revoked"` — при следующем `/validate` доступ пропадёт.
- **Срок:** `"expires": "2026-12-31T23:59:59Z"` (ISO 8601) — пусто = бессрочно.
- **Лимит устройств:** `max_devices` (на ключ). Привязки лежат в `devices` в том же файле.

---

## 10. Безопасность (улучшения после первого запуска)
- Перенести `ED25519_PRIVATE_B64` и `S3_SECRET` из переменных функции в **Lockbox**
  (секрет-менеджер) и подключить как секреты функции — чтобы они не светились в настройках.
- **Фаза 2 в клиенте:** встроить `public.pem` и проверять подпись токена офлайн (Ed25519),
  чтобы локальный `license.dat` нельзя было подделать; добавить периодический `/validate` (heartbeat).
- Подписать установщик приложения сертификатом разработчика.

---

## Частые ошибки
- **403 на /activate с правильным ключом:** проверьте, что `state.json` действительно лежит в бакете
  и `S3_BUCKET` совпадает; что у сервис-аккаунта роль `storage.editor`.
- **500 / no token:** скорее всего неверный `ED25519_PRIVATE_B64` (перенос строк) — перегенерируйте base64 без переносов.
- **API Gateway 500:** у сервис-аккаунта интеграции нет роли `functions.functionInvoker`.
- **YDB вместо файла:** при росте числа лицензий замените Object Storage на YDB (serverless) —
  логика та же, меняется только слой чтения/записи состояния.

---

## Автообновление приложения

Приложение само проверяет новую версию через `version.json` в Object Storage и, если она
новее, скачивает установщик и запускает его. Реализовано в `src/updater.cpp`
(`Update::kManifestUrl` — адрес манифеста).

**Разовая настройка:**
1. Создать **отдельный публичный** бакет для обновлений:
   ```bash
   yc storage bucket create --name smartview-updates --public-read
   ```
   (или в консоли включить публичное чтение объектов). Файлы обновлений не секретны.
2. Убедиться, что в `src/updater.h` `kManifestUrl` указывает на
   `https://storage.yandexcloud.net/smartview-updates/version.json`.

**Выпуск новой версии:**
1. Поднять `#define VERSION` в `src/updater.h` (например `"1.0.1"`), собрать приложение и
   установщик (твоя обычная сборка QtIFW), назвать его, например, `SmartView-1.0.1-setup.exe`.
2. Опубликовать одной командой:
   ```powershell
   cd C:\Users\kulak\Desktop\SmartView\server
   .\publish-update.ps1 -Version 1.0.1 -File "C:\build\SmartView-1.0.1-setup.exe" -Notes "Что нового"
   ```
   Скрипт зальёт установщик в бакет и обновит `version.json`.
3. У установленных пользователей при следующем запуске появится предложение обновиться →
   приложение скачает установщик и запустит его.

Версия в `version.json` должна совпадать с `VERSION` нового билда. Манифест и установщик —
публичные (обновления не привязаны к лицензии); запуск самой программы по-прежнему требует ключа.
