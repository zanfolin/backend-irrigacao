import "dotenv/config"; // Carrega as variáveis do .env
import express from "express";
import routes from "./routes.js";
import "./database.js"; // Importa para inicializar o banco

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware para interpretar JSON no corpo das requisições
app.use(express.json());

// Prefixa todas as rotas com '/api'
app.use("/api", routes);

// Rota raiz de boas-vindas
app.get("/", (req, res) => {
  res.send("API de Irrigação no ar! 🚀");
});

// Inicia o servidor
app.listen(PORT, () => {
  console.log(`Servidor rodando em http://localhost:${PORT}`);
});
