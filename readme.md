# Backend de Irrigação (Node.js + Express + SQLite)

Este é um backend simples em Node.js e Express para gerenciar um sistema de irrigação. Ele utiliza um banco de dados SQLite interno para armazenar o histórico de umidade, controlar o acionamento da água e gerenciar a autenticação de usuários com JSON Web Tokens (JWT).

## 🚀 Tecnologias Utilizadas

  * **Node.js:** (v22+ recomendado)
  * **Express:** (Gerenciamento de rotas e servidor)
  * **SQLite3:** (Banco de dados SQL leve e interno)
  * **jsonwebtoken:** (Para autenticação JWT)
  * **bcryptjs:** (Para hashing de senhas)
  * **dotenv:** (Para gerenciar variáveis de ambiente)

## 1\. Pré-requisitos

Antes de começar, garanta que você tenha os seguintes softwares instalados:

1.  **Node.js:** Versão 22 ou superior.
2.  **npm:** (Normalmente instalado junto com o Node.js).
3.  **Um Cliente de API:** (Recomendado) Postman, Insomnia ou similar para testar as rotas.

## 2\. Instalação e Configuração

Siga estes passos para configurar e rodar o projeto localmente.

### Passo 1: Obter os Arquivos

Clone este repositório ou baixe e descompacte os arquivos em um diretório de sua escolha (ex: `projeto-irrigacao`).

### Passo 2: Instalar as Dependências

Abra seu terminal, navegue até a pasta raiz do projeto e execute o comando:

```bash
npm install
```

Este comando irá baixar todas as bibliotecas listadas no `package.json` (Express, SQLite, JWT, etc.).

### Passo 3: Configurar Variáveis de Ambiente

Na raiz do projeto, crie um arquivo chamado `.env`. Este arquivo guardará seu segredo JWT.

Adicione o seguinte conteúdo a ele:

```
# Mude "seu_segredo_super_secreto_aqui" para qualquer frase complexa.
# Isso é usado para assinar e verificar os tokens JWT.
JWT_SECRET=seu_segredo_super_secreto_aqui_123456
```

## 3\. Rodando a Aplicação

Com tudo instalado, você pode iniciar o servidor.

### Modo de Desenvolvimento

Para rodar o servidor em modo de desenvolvimento (que reinicia automaticamente sempre que você salva uma alteração no código), use:

```bash
npm run dev
```

### Modo de Produção

Para rodar o servidor em modo de produção (mais performático e sem reinício automático), use:

```bash
npm start
```

### Verificando o Sucesso

Se tudo correu bem, você verá as seguintes mensagens no seu terminal:

```
Servidor rodando em http://localhost:3000
Conectado ao banco de dados SQLite.
Banco de dados inicializado com sucesso.
```

> **Nota:** Ao iniciar pela primeira vez, um arquivo chamado `database.sqlite` será criado automaticamente na raiz do projeto. Este é o seu banco de dados.

-----

## 4\. Como Usar (Testando a API)

Todas as rotas da API são prefixadas com `/api`. A URL base para seus testes será: `http://localhost:3000/api`

### Fluxo de Teste Recomendado

Você precisará de um **Token de Autenticação** para acessar a maioria das rotas. Siga este fluxo:

#### Parte 1: Autenticação (Rotas Públicas)

Use o Postman ou similar para fazer estas requisições:

**1. Criar um Usuário (Signup)**

  * **Método:** `POST`
  * **URL:** `http://localhost:3000/api/signup`
  * **Corpo (Body) -\> (raw - JSON):**

<!-- end list -->

```json
{
  "usuario": "meu_usuario",
  "senha": "minha_senha_123"
}
```

**2. Fazer Login (Signin)**

  * **Método:** `POST`
  * **URL:** `http://localhost:3000/api/signin`
  * **Corpo (Body) -\> (raw - JSON):**

<!-- end list -->

```json
{
  "usuario": "meu_usuario",
  "senha": "minha_senha_123"
}
```

  * **Resposta:** Você receberá um JSON contendo o token:

<!-- end list -->

```json
{
  "message": "Login bem-sucedido!",
  "token": "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "user": {
    "id": 1,
    "usuario": "meu_usuario"
  }
}
```

**➡️ Copie o valor do `token` (incluindo o "Bearer ") para usar no próximo passo.**

-----

#### Parte 2: Rotas Protegidas (Exigem Token)

Para todas as requisições abaixo, você deve configurar o **Header de Autenticação** no seu cliente de API:

  * **Tipo de Auth:** `Bearer Token`
  * **Header Key:** `Authorization`
  * **Header Value:** `Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...` (Cole o token que você copiou)

**3. Registrar Umidade**

  * **Método:** `POST`
  * **URL:** `http://localhost:3000/api/umidade/85` (O valor `85` pode ser qualquer número de 0 a 100)

**4. Ver Histórico de Umidade**

  * **Método:** `GET`
  * **URL:** `http://localhost:3000/api/historicoumidade`
  * *Retorna um JSON com os últimos 15 registros de umidade.*

**5. Ligar/Desligar Água**

  * **Método:** `POST`
  * **URL:** `http://localhost:3000/api/agua`
  * *Esta rota inverte o estado atual da água (se estava 0, vira 1; se estava 1, vira 0).*

**6. Verificar Estado da Água**

  * **Método:** `GET`
  * **URL:** `http://localhost:3000/api/agua`
  * *Retorna o último estado (ex: `{"id": 1, "ligada": 1, ...}`).*

**7. Obter Dados do Usuário Logado**

  * **Método:** `GET`
  * **URL:** `http://localhost:3000/api/usuario`
  * *Retorna os dados do usuário dono do token.*

**8. Fazer Logout (Signout)**

  * **Método:** `GET`
  * **URL:** `http://localhost:3000/api/signout`
  * *Apenas retorna uma mensagem informativa. No lado do cliente (frontend), você deve apagar o token que estava armazenado.*