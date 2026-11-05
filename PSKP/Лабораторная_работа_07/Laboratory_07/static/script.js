document.addEventListener("DOMContentLoaded", () => {
    const jsonContainer = document.getElementById('json-container');
    const xmlContainer = document.getElementById('xml-container');

    function displayData(element, data) {
        element.innerText = data;
        element.classList.remove('loading');
    }

    fetch('/data.json')
        .then(response => response.json())
        .then(data => {
            setTimeout(() => {
                displayData(jsonContainer, JSON.stringify(data, null, 2));
            }, 300);
        })
        .catch(err => {
            console.error('Ошибка загрузки JSON', err);
            displayData(jsonContainer, 'Ошибка загрузки JSON');
        });

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