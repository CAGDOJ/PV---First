# WRENCH PV-first Energy Infrastructure  
# Infraestrutura Energética PV-first para o Simulador WRENCH

---

## 1. Visão Geral | Overview

### 🇧🇷 Português
Este projeto implementa uma **extensão de infraestrutura** para o simulador
**WRENCH (Workflow Simulation Workbench)**, com o objetivo de incorporar
**modelos energéticos**, **políticas de decisão baseadas em energia renovável**
e **métricas ambientais** diretamente no núcleo da simulação.

A principal contribuição é a implementação nativa da política energética
**PV-first**, que prioriza o uso de energia solar fotovoltaica antes do consumo
de energia da rede elétrica, permitindo a avaliação integrada de:
- consumo energético,
- uso de energia renovável,
- pegada hídrica indireta associada à geração elétrica.

O projeto é desenvolvido em **C++**, como parte de uma pesquisa de **mestrado em
Computação**, com foco em **engenharia de simuladores** e **infraestrutura de
simulação energética para HPC**.

---

### 🇺🇸 English
This project implements a **simulation infrastructure extension** for the
**WRENCH (Workflow Simulation Workbench)** simulator, aiming to incorporate
**energy models**, **renewable-aware decision policies**, and **environmental
metrics** directly into the simulation core.

The main contribution is the native implementation of the **PV-first energy
policy**, which prioritizes photovoltaic solar energy before drawing electricity
from the power grid, enabling integrated evaluation of:
- energy consumption,
- renewable energy usage,
- indirect water footprint associated with electricity generation.

The project is developed in **C++** as part of a **master’s research in Computer
Science**, focusing on **simulation infrastructure engineering** for HPC systems.

---

## 2. Motivação da Pesquisa | Research Motivation

### 🇧🇷 Português
Simuladores de HPC, como o WRENCH, são amplamente utilizados para avaliar
desempenho, escalonamento e comportamento de workflows distribuídos. Entretanto,
o suporte nativo a **políticas energéticas baseadas em fontes renováveis** e a
**métricas ambientais não tradicionais**, como a pegada hídrica, ainda é limitado.

Este trabalho parte da premissa de que a sustentabilidade de sistemas de HPC não
deve ser analisada apenas sob a ótica de desempenho ou emissões de carbono, mas
também considerando o **impacto indireto sobre recursos hídricos**, fortemente
associado à geração termelétrica.

---

### 🇺🇸 English
HPC simulators such as WRENCH are widely used to evaluate performance,
scheduling, and workflow behavior. However, native support for
**renewable-based energy policies** and **non-traditional environmental
metrics**, such as water footprint, is still limited.

This work is motivated by the premise that HPC sustainability should not be
analyzed solely from a performance or carbon perspective, but also considering
the **indirect impact on water resources**, strongly linked to thermoelectric
power generation.

---

## 3. Objetivo | Objective

### 🇧🇷 Português
O objetivo deste projeto é **estender a infraestrutura do simulador WRENCH**
para suportar:
- modelagem de geração fotovoltaica,
- modelagem de consumo energético de nós HPC,
- políticas energéticas baseadas em renováveis (PV-first),
- cálculo da energia proveniente da rede elétrica,
- estimativa da pegada hídrica indireta.

A proposta não consiste apenas em utilizar o WRENCH, mas em **modificar e
expandir sua infraestrutura**, caracterizando um trabalho de **engenharia de
software científico**.

---

### 🇺🇸 English
The objective of this project is to **extend the WRENCH simulation
infrastructure** to support:
- photovoltaic energy generation modeling,
- HPC node energy consumption modeling,
- renewable-aware energy policies (PV-first),
- computation of grid energy usage,
- estimation of indirect water footprint.

Rather than simply using WRENCH, this work **modifies and expands its
infrastructure**, characterizing a **scientific software engineering** effort.

---

## 4. Escopo | Scope

### 🇧🇷 Português

Incluído no escopo:
- Implementação em C++ integrada ao WRENCH
- Modelos energéticos acoplados ao tempo de simulação
- Política PV-first aplicada durante a execução dos workflows
- Coleta de métricas energéticas e ambientais

Fora do escopo:
- Implementação de sistema operacional
- Medições em hardware real
- Otimização de desempenho do WRENCH
- Previsão meteorológica avançada
- Armazenamento energético (baterias)

---

### 🇺🇸 English

Included in scope:
- C++ implementation integrated into WRENCH
- Energy models coupled with simulation time
- PV-first policy applied during workflow execution
- Collection of energy and environmental metrics

Out of scope:
- Operating system implementation
- Real hardware measurements
- WRENCH performance optimization
- Advanced weather forecasting
- Energy storage systems (batteries)

---

## 5. Arquitetura da Extensão | Extension Architecture

### 🇧🇷 Português
A arquitetura da extensão segue um modelo modular, mantendo separação clara de
responsabilidades:

- **energy/**  
  Modelos de geração fotovoltaica e consumo energético do HPC

- **policy/**  
  Políticas de decisão energética (PV-first)

- **metrics/**  
  Métricas ambientais, com foco na pegada hídrica indireta

- **simulation/**  
  Camada de integração com o núcleo do WRENCH

---

### 🇺🇸 English
The extension architecture follows a modular design, ensuring clear separation
of concerns:

- **energy/**  
  Photovoltaic generation and HPC energy consumption models

- **policy/**  
  Energy decision policies (PV-first)

- **metrics/**  
  Environmental metrics, focusing on indirect water footprint

- **simulation/**  
  Integration layer with the WRENCH core

---

## 6. Estrutura do Projeto | Project Structure

```text
wrench-pvfirst/
│
├── src/
│   ├── energy/
│   │   ├── PVModel.hpp
│   │   ├── PVModel.cpp
│   │   ├── HPCPowerModel.hpp
│   │   └── HPCPowerModel.cpp
│   │
│   ├── policy/
│   │   ├── PVFirstPolicy.hpp
│   │   └── PVFirstPolicy.cpp
│   │
│   ├── metrics/
│   │   ├── WaterFootprint.hpp
│   │   └── WaterFootprint.cpp
│   │
│   └── simulation/
│       ├── EnergyManager.hpp
│       └── EnergyManager.cpp
│
├── examples/
│   └── pvfirst_example.cpp
│
├── CMakeLists.txt
└── README.md