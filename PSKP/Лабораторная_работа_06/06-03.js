if (process.argv.includes("--direct")) process.env.MAIL_MODE = "direct";
const { send } = require("./m0603");

const message = process.argv.slice(2).filter(arg => arg !== "--direct").join(" ") ||
    "Проверка приложения 06-03: функция send принимает одну строку.";

send(message)
    .then(result => {
        console.log("06-03: модуль из папки ./m0603");
        console.log(`Получатель: ${result.to}`);
        console.log(`Сообщение: ${message}`);
        console.log(`Ответ SMTP: ${result.reply}`);
        console.log(result.mode === "local"
            ? "Письмо принято локально: http://localhost:1080"
            : "Письмо принято почтовым сервером; проверь почту получателя.");
    })
    .catch(error => {
        console.error(`Ошибка: ${error.message}`);
        process.exitCode = 1;
    });
