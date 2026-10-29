document.addEventListener("DOMContentLoaded", () => {
    const jsonContainer = document.getElementById('json-container');
    const xmlContainer = document.getElementById('xml-container');

    // Функция для красивого вывода данных с плавным появлением
    function displayData(element, data) {
        element.innerText = data;
        element.classList.remove('loading'); // Убираем прозрачность
    }

    // Загрузка JSON 
    fetch('/data.json')
        .then(response => response.json())
        .then(data => {
            // setTimeout используем просто для красивой демонстрации анимации
            setTimeout(() => {
                displayData(jsonContainer, JSON.stringify(data, null, 2));
            }, 300);
        })
        .catch(err => {
            console.error('Ошибка загрузки JSON', err);
            displayData(jsonContainer, 'Ошибка загрузки JSON');
        });

    // Загрузка XML
    fetch('/data.xml')
        .then(response => response.text())
        .then(data => {
            setTimeout(() => {
                displayData(xmlContainer, data);
            }, 600);
        })
        .catch(err => {
            console.error('Ошибка загрузки XML', err);
            displayData(xmlContainer, 'Ошибка загрузки XML');
        });
});