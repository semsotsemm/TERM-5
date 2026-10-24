const { EMAIL } = require("./config");
const { sendMessage } = require("./mailer");

// Публичный интерфейс модуля: одна функция, один параметр — строка.
async function send(text) {
    if (typeof text !== "string" || !text.trim()) {
        throw new TypeError("send принимает непустую строку.");
    }
    if (process.env.MAIL_MODE === "direct" && EMAIL === "your-email@gmail.com") {
        throw new Error("Сначала впиши свой адрес в константу EMAIL в m0603/config.js.");
    }
    return sendMessage({
        from: EMAIL,
        to: EMAIL,
        subject: "ПСКП — лабораторная работа 06, модуль m0603",
        text
    });
}

module.exports = { send };
