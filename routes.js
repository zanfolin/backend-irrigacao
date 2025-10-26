import { Router } from "express";
import * as authController from "./app/controllers/authController.js";
import * as umidadeController from "./app/controllers/umidadeController.js";
import * as aguaController from "./app/controllers/aguaController.js";
import { authenticateToken } from "./app/middleware/authMiddleware.js";

const router = Router();

// --- Rotas de Autenticação (Públicas) ---
router.post("/signup", authController.signup);
router.post("/signin", authController.signin); // Corrigido de GET para POST
router.get("/signout", authController.signout);

// --- Rotas de Usuário (Protegida) ---
router.get("/usuario", authenticateToken, authController.getUser);

// --- Rotas de Irrigação (Protegidas) ---
router.post(
  "/umidade/:umidade",
  authenticateToken,
  umidadeController.addUmidade
);
router.get(
  "/historicoumidade",
  authenticateToken,
  umidadeController.getHistorico
);

router.post("/agua", authenticateToken, aguaController.setAgua);
router.get("/agua", authenticateToken, aguaController.getAgua);

export default router;
