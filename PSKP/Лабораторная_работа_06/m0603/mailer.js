const createSendmail = require("sendmail");

function isEmail(value) {
    return typeof value === "string" &&
        /^[A-Za-z0-9.!#$%&'*+\-\/=\?^_`{|}~]+@[A-Za-z0-9](?:[A-Za-z0-9.-]*[A-Za-z0-9])?\.[A-Za-z]{2,}$/.test(value);
}

function sendMessage(mail) {
    if (!isEmail(mail.from) || !isEmail(mail.to)) {
        return Promise.reject(new Error("Укажи по одному корректному email отправителя и получателя."));
    }
    if (typeof mail.text !== "string" || !mail.text.trim() || mail.text.length > 10000) {
        return Promise.reject(new Error("Сообщение должно содержать от 1 до 10000 символов."));
    }
    const direct = process.env.MAIL_MODE === "direct";
    // devHost/devPort — штатные настройки sendmail для локального SMTP.
    // В direct-режиме пакет сам ищет MX и подключается к порту 25.
    const options = direct
        ? { silent: true }
        : { silent: true, devHost: "127.0.0.1", devPort: 1025 };
    const sendmail = createSendmail(options);
    return new Promise((resolve, reject) => {
        sendmail(mail, (error, reply) => {
            if (error) {
                if (!direct && error.message.includes("ECONNREFUSED")) {
                    reject(new Error("SMTP-приёмник недоступен. Сначала запусти node 06-01.js в другом терминале."));
                } else {
                    reject(error);
                }
                return;
            }
            resolve({
                from: mail.from, to: mail.to, subject: mail.subject,
                mode: direct ? "direct" : "local", reply: String(reply).trim()
            });
        });
    });
}

module.exports = { sendMessage, isEmail };
