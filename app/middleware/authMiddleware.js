import jwt from 'jsonwebtoken';

export const authenticateToken = (req, res, next) => {
  const authHeader = req.headers['authorization'];
  // O token vem no formato "Bearer TOKEN"
  const token = authHeader && authHeader.split(' ')[1];

  if (token == null) {
    // 401 Unauthorized
    return res.status(401).json({ message: "Token de acesso não fornecido." });
  }

  jwt.verify(token, process.env.JWT_SECRET, (err, user) => {
    if (err) {
      // 403 Forbidden
      return res.status(403).json({ message: "Token inválido ou expirado." });
    }
    // Adiciona os dados do usuário (payload do token) ao objeto 'req'
    req.user = user;
    next();
  });
};