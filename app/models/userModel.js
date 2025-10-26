import db from "../../database.js";

export const findByUsuario = (usuario) => {
  return new Promise((resolve, reject) => {
    db.get(
      "SELECT * FROM usuarios WHERE usuario = ?",
      [usuario],
      (err, row) => {
        if (err) reject(err);
        resolve(row);
      }
    );
  });
};

export const findById = (id) => {
  return new Promise((resolve, reject) => {
    // Nunca retorne a senha, mesmo que hasheada
    db.get(
      "SELECT id, usuario, created_at, updated_at FROM usuarios WHERE id = ?",
      [id],
      (err, row) => {
        if (err) reject(err);
        resolve(row);
      }
    );
  });
};

export const create = (usuario, senhaHash) => {
  return new Promise((resolve, reject) => {
    db.run(
      "INSERT INTO usuarios (usuario, senha) VALUES (?, ?)",
      [usuario, senhaHash],
      function (err) {
        if (err) reject(err);
        // Retorna o ID do usuário criado
        resolve({ id: this.lastID, usuario });
      }
    );
  });
};
