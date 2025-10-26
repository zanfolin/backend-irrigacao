import db from "../../database.js";

export const create = (valor) => {
  return new Promise((resolve, reject) => {
    db.run("INSERT INTO umidade (valor) VALUES (?)", [valor], function (err) {
      if (err) reject(err);
      resolve({ id: this.lastID, valor });
    });
  });
};

export const findLast15 = () => {
  return new Promise((resolve, reject) => {
    db.all(
      "SELECT * FROM umidade ORDER BY created_at DESC LIMIT 15",
      [],
      (err, rows) => {
        if (err) reject(err);
        resolve(rows);
      }
    );
  });
};
