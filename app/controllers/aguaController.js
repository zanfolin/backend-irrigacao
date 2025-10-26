import * as aguaModel from "../models/aguaModel.js";

// POST /agua
export const setAgua = async (req, res) => {
  try {
    // Esta rota vai simplesmente "inverter" o último estado da água.
    // Se estava desligada (0), liga (1). Se estava ligada (1), desliga (0).
    const ultimoEstado = await aguaModel.findLast();

    // Se nunca houve registro, consideramos o estado inicial como "desligado" (0)
    const estadoAtual = ultimoEstado ? ultimoEstado.ligada : 0;

    // Inverte o estado (0 vira 1, 1 vira 0)
    const novoEstado = !estadoAtual;

    const registro = await aguaModel.create(novoEstado);

    res.status(201).json({
      message: `Irrigação ${novoEstado ? "LIGADA" : "DESLIGADA"}.`,
      data: registro,
    });
  } catch (error) {
    res.status(500).json({ message: "Erro no servidor", error: error.message });
  }
};

// GET /agua
export const getAgua = async (req, res) => {
  try {
    const ultimoEstado = await aguaModel.findLast();

    if (!ultimoEstado) {
      return res.json({
        ligada: 0,
        message: "Nenhum registro encontrado, assumindo 'desligada'.",
      });
    }

    // Retorna o último registro (ligada: 1 ou 0)
    res.json(ultimoEstado);
  } catch (error) {
    res.status(500).json({ message: "Erro no servidor", error: error.message });
  }
};
