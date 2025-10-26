import bcrypt from "bcryptjs";
import jwt from "jsonwebtoken";
import * as userModel from "../models/userModel.js";

// POST /signup
export const signup = async (req, res) => {
  try {
    const { usuario, senha } = req.body;
    if (!usuario || !senha) {
      return res
        .status(400)
        .json({ message: "Usuário e senha são obrigatórios." });
    }

    const existingUser = await userModel.findByUsuario(usuario);
    if (existingUser) {
      return res.status(409).json({ message: "Este usuário já existe." });
    }

    const senhaHash = await bcrypt.hash(senha, 10);
    const newUser = await userModel.create(usuario, senhaHash);

    res
      .status(201)
      .json({ message: "Usuário criado com sucesso!", user: newUser });
  } catch (error) {
    res.status(500).json({ message: "Erro no servidor", error: error.message });
  }
};

// POST /signin (Nota: Mudei de GET para POST, que é o correto para enviar body)
export const signin = async (req, res) => {
  try {
    const { usuario, senha } = req.body;
    if (!usuario || !senha) {
      return res
        .status(400)
        .json({ message: "Usuário e senha são obrigatórios." });
    }

    const user = await userModel.findByUsuario(usuario);
    if (!user) {
      return res.status(404).json({ message: "Usuário não encontrado." });
    }

    const isMatch = await bcrypt.compare(senha, user.senha);
    if (!isMatch) {
      return res.status(401).json({ message: "Senha inválida." });
    }

    // Cria o token JWT
    const payload = { id: user.id, usuario: user.usuario };
    const token = jwt.sign(payload, process.env.JWT_SECRET, {
      expiresIn: "1h", // Token expira em 1 hora
    });

    res.json({
      message: "Login bem-sucedido!",
      token: `Bearer ${token}`,
      user: payload,
    });
  } catch (error) {
    res.status(500).json({ message: "Erro no servidor", error: error.message });
  }
};

// GET /usuario (Protegido)
export const getUser = async (req, res) => {
  try {
    // req.user é injetado pelo middleware 'authenticateToken'
    const user = await userModel.findById(req.user.id);
    if (!user) {
      return res
        .status(404)
        .json({ message: "Usuário do token não encontrado." });
    }
    res.json(user);
  } catch (error) {
    res.status(500).json({ message: "Erro no servidor", error: error.message });
  }
};

// GET /signout
export const signout = (req, res) => {
  // Em um sistema JWT stateless, o "signout" é responsabilidade do cliente.
  // O cliente deve apenas apagar/invalidar o token do seu armazenamento.
  res.json({
    message: "Logout realizado. Por favor, apague o token no cliente.",
  });
};
