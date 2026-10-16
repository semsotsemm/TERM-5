const EventEmitter = require("events");

class DB extends EventEmitter
{
    constructor()
    {
        super();
        this.db_data = [
            { id: 1, name: "Иванов И.И.", bday: "2001-01-01" },
            { id: 2, name: "Петров П.П.", bday: "2002-02-02" },
            { id: 3, name: "Сидоров С.С.", bday: "2003-03-03" }
        ];
    }

    select(callback)
    {
        callback(null, this.db_data);
    }

    insert(row, callback)
    {
        if (!row.id || !row.name || !row.bday)
        {
            return callback("Не все поля (id, name, bday) заполнены", null);
        }

        const exists = this.db_data.find(item => item.id == row.id);
        if (exists)
        {
            return callback(`Запись с id ${row.id} уже существует`, null);
        }

        this.db_data.push(row);
        callback(null, row);
    }

    update(row, callback)
    {
        const index = this.db_data.findIndex(item => item.id == row.id);
        if (index === -1)
        {
            return callback(`Запись с id ${row.id} не найдена`, null);
        }

        this.db_data[index] = row;
        callback(null, row);
    }

    delete(id, callback)
    {
        const index = this.db_data.findIndex(item => item.id == id);
        if (index === -1)
        {
            return callback(null, null);
        }

        const deleted_row = this.db_data.splice(index, 1)[0];
        callback(null, deleted_row);
    }

    commit(callback)
    {
        // Имитация выполнения фиксации состояния БД
        if (callback)
        {
            callback(null, true);
        }
    }
}

const db = new DB();

db.on("GET", (callback) => {
    db.select(callback);
});

db.on("POST", (row, callback) => {
    db.insert(row, callback);
});

db.on("PUT", (row, callback) => {
    db.update(row, callback);
});

db.on("DELETE", (id, callback) => {
    db.delete(id, callback);
});

db.on("COMMIT", (callback) => {
    db.commit(callback);
});

module.exports = db;