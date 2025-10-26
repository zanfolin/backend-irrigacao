import db from "../../database.js";

export const findLast = () => {
  return new Promise((resolve, reject) => {
    db.get(
      "SELECT * FROM agua ORDER BY created_at DESC LIMIT 1",
      [],
      (err, row) => {
        if (err) reject(err);
        resolve(row);
      }
    );
  });
};

export const create = (ligada) => {
  return new Promise((resolve, reject) => {
    // Armazena 1 para true, 0 para false
    const valor = ligada ? 1 : 0;
    db.run("INSERT INTO agua (ligada) VALUES (?)", [valor], function (err) {
      if (err) reject(err);
      resolve({ id: this.lastID, ligada: valor });
    });
  });
};
