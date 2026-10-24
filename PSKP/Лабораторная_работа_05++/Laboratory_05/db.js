const EventEmitter = require("events");

let dbArray = [
    { "id": 1, "name": "Иванов Иван Иванович", "bday": "12.06.2005" },
    { "id": 2, "name": "Петров Петр Петрович", "bday": "18.09.2005" },
    { "id": 3, "name": "Никитин Никита Никитьич", "bday": "22.12.2005" },
    { "id": 4, "name": "Алексеев Алексей Алексеевич", "bday": "17.04.2005" },
    { "id": 5, "name": "Александров Александр Александрович", "bday": "01.01.2005" }
];

class DB extends EventEmitter {
    constructor() {
        super();

        this.on("GET", async (id, callback) => {
            try {
                if (id) {
                    const data = await this.SelectRowById(id);
                    callback(null, data);
                } else {
                    const data = await this.SelectAllRows();
                    callback(null, data);
                }
            } catch (error) {
                callback(error, null);
            }
        });

        this.on("POST", async (request_body, callback) => {
            try {
                const inserted_row = await this.InsertNewRow(request_body);
                callback(null, inserted_row);
            } catch (error) {
                callback(error, null);
            }
        });

        this.on("PUT", async (request_body, callback) => {
            try {
                const updated_row = await this.UpdateRow(request_body);
                callback(null, updated_row);
            } catch (error) {
                callback(error, null);
            }
        });

        this.on("DELETE", async (row_id, callback) => {
            try {
                const deleted = await this.DeleteRow(row_id);
                callback(null, deleted);
            } catch (error) {
                callback(error, null);
            }
        });

        this.on("COMMIT", async () => {
            await this.commit();
        });
    }

    async SelectAllRows() {
        return dbArray;
    }

    async InsertNewRow(row) {
        const max_index = dbArray.length > 0 ? Math.max(...dbArray.map(item => item.id)) : 0;
        row.id = max_index + 1;
        dbArray.push(row);
        return row;
    }

    async UpdateRow(new_row) {
        const index = dbArray.findIndex(item => item.id === new_row.id);
        if (index > -1) {
            dbArray[index] = new_row;
            return new_row;
        }
        return null;
    }

    async SelectRowById(row_id) {
        const row = dbArray.find(item => item.id === Number(row_id));
        return row ? row : null;
    }

    async DeleteRow(row_id) {
        const index = dbArray.findIndex(item => item.id === Number(row_id));
        if (index > -1) {
            const deleted_row = dbArray.splice(index, 1)[0];
            return deleted_row;
        }
        return null;
    }

    async commit() {
        console.log("[DB] Фиксация состояния бд.");
    }
}

module.exports = new DB();