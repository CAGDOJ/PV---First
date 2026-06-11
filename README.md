## Pré-requisito no Windows: WSL com Ubuntu

Este projeto foi desenvolvido para ser compilado e executado em ambiente Linux, utilizando ferramentas como `bash`, `make`, `cmake`, `g++` e bibliotecas do SimGrid.

No Windows, recomenda-se utilizar o **WSL — Windows Subsystem for Linux**, com a distribuição **Ubuntu** instalada.

### 1. Verificar se o WSL já está instalado

Abra o **PowerShell** e execute:

```powershell
wsl -l -v
```

Se aparecer uma distribuição como `Ubuntu` ou `Ubuntu-24.04`, o WSL já está instalado.

Exemplo:

```text
NAME            STATE           VERSION
Ubuntu          Stopped         2
```

Caso apareça uma mensagem informando que não há distribuições instaladas, será necessário instalar o Ubuntu.

### 2. Instalar o WSL com Ubuntu

Abra o **PowerShell como Administrador** e execute:

```powershell
wsl --install -d Ubuntu
```

Se necessário, também é possível listar as distribuições disponíveis com:

```powershell
wsl --list --online
```

E instalar uma versão específica, por exemplo:

```powershell
wsl --install -d Ubuntu-24.04
```

Após a instalação, reinicie o computador.

### 3. Configurar o Ubuntu pela primeira vez

Depois de reiniciar, abra o menu iniciar do Windows e procure por:

```text
Ubuntu
```

Na primeira execução, o Ubuntu solicitará a criação de um usuário e senha.

Exemplo de usuário:

```text
carlos
```

A senha não aparece enquanto é digitada. Isso é normal.

### 4. Acessar o projeto pelo terminal Ubuntu

Considerando que o projeto esteja na pasta:

```text
C:\Users\oliveiracagoj\Documents\Nova pasta\PV---First\pvfirst
```

No terminal Ubuntu/WSL, acesse o diretório usando o caminho Linux correspondente:

```bash
cd "/mnt/c/Users/oliveiracagoj/Documents/Nova pasta/PV---First/pvfirst"
```

O uso de aspas é necessário porque o caminho possui espaço em `Nova pasta`.

### 5. Instalar as dependências do projeto

Dentro do terminal Ubuntu/WSL, execute:

```bash
sudo apt update
sudo apt install -y build-essential cmake make g++ gcc libsimgrid-dev
```

Esses pacotes instalam as ferramentas necessárias para compilar o projeto C++ com CMake e SimGrid.

### 6. Corrigir permissões dos scripts

Como o projeto pode ter sido editado no Windows, é recomendado corrigir as quebras de linha e dar permissão de execução aos scripts:

```bash
sed -i 's/\r$//' run.sh
sed -i 's/\r$//' run_daylight.sh

chmod +x run.sh
chmod +x run_daylight.sh
```

### 7. Compilar o projeto

Execute os comandos abaixo:

```bash
rm -rf build
mkdir build
cd build
cmake ..
make
```

### 8. Executar o projeto

Após a compilação, volte para a raiz do projeto:

```bash
cd ..
```

Execute o cenário principal:

```bash
./run.sh
```

Ou execute o cenário daylight:

```bash
./run_daylight.sh
```

### Observação importante

Os comandos abaixo são comandos Linux e devem ser executados no terminal Ubuntu/WSL:

```bash
sudo apt update
make
cmake
./run.sh
```

Eles não devem ser executados diretamente no PowerShell do Windows, pois o PowerShell não reconhece comandos como `sudo` e `apt`.

Caso apareça o erro:

```text
sudo não é reconhecido como nome de cmdlet
```

significa que o comando foi executado no PowerShell, e não no terminal Ubuntu/WSL.
