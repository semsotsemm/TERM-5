const path = require("node:path");
const { execSync } = require("node:child_process");
const { createRequire } = require("node:module");
const packageName = require("./m0603/package.json").name;

if (process.argv.includes("--direct")) process.env.MAIL_MODE = "direct";

function LoadInstalledPackage() {
    if (!process.argv.includes("--global")) {
        try {
            return { module: require(packageName), file: require.resolve(packageName), source: "локальная установка" };
        } catch (error) {
            if (error.code !== "MODULE_NOT_FOUND" ||
                !error.message.startsWith(`Cannot find module '${packageName}'`)) throw error;
        }
    }
    // Обычный require не ищет npm-пакеты в глобальной папке автоматически.
    const globalRoot = execSync("npm root -g", { encoding: "utf8", windowsHide: true }).trim();
    const globalRequire = createRequire(path.join(globalRoot, "__lab06__.js"));
    return {
        module: globalRequire(packageName), file: globalRequire.resolve(packageName),
        source: "глобальная установка"
    };
}

async function main() {
    const { module: { send }, file, source } = LoadInstalledPackage();
    console.log(`06-04: ${source}`);
    console.log(`Пакет: ${packageName}`);
    console.log(`Загружен файл: ${file}`);
    const message = process.argv.slice(2).filter(arg => !["--direct", "--global"].includes(arg)).join(" ") ||
        "Проверка приложения 06-04: send из установленного npm-пакета.";
    const result = await send(message);
    console.log(`Получатель: ${result.to}`);
    console.log(`Сообщение: ${message}`);
    console.log(`Ответ SMTP: ${result.reply}`);
    console.log(result.mode === "local" ? "Письмо принято локально: http://localhost:1080" : "Письмо принято почтовым сервером.");
}

main().catch(error => {
    console.error(`Ошибка: ${error.message}`);
    console.error(`Установи свой пакет ${packageName} локально или глобально.`);
    process.exitCode = 1;
});
