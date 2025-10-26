import * as umidadeModel from "../models/umidadeModel.js";

// POST /umidade/:umidade
export const addUmidade = async (req, res) => {
  try {
    const { umidade } = req.params;
    const valor = parseInt(umidade, 10);

    if (isNaN(valor) || valor < 0 || valor > 100) {
      return res
        .status(400)
        .json({
          message:
            "Valor de umidade inválido. Deve ser um inteiro entre 0 e 100.",
        });
    }

    const novoRegistro = await umidadeModel.create(valor);
    res
      .status(201)
      .json({ message: "Umidade registrada.", data: novoRegistro });
  } catch (error) {
    res.status(500).json({ message: "Erro no servidor", error: error.message });
  }
};

// GET /historicoumidade
export const getHistorico = async (req, res) => {
  try {
    const historico = await umidadeModel.findLast15();
    res.json(historico);
  } catch (error) {
    res.status(500).json({ message: "Erro no servidor", error: error.message });
  }
};
