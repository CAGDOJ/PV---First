# PV-first HPC Energy Simulator  
# Simulador Energético para HPC com Política PV-first

---

## 1. Visão Geral | Overview

### 🇧🇷 Português
Este projeto implementa um **simulador energético** para plataformas de
Computação de Alto Desempenho (High Performance Computing – HPC), com foco
na integração de **energia solar fotovoltaica** por meio da política
energética **PV-first**.

A política PV-first prioriza o uso da energia fotovoltaica gerada localmente
em cada intervalo de tempo, recorrendo à rede elétrica convencional apenas
quando a geração solar é insuficiente para suprir a demanda energética do
sistema HPC.

O simulador foi concebido como o **artefato primário de uma pesquisa de
mestrado**, servindo como base computacional para análises científicas
relacionadas à sustentabilidade energética e à **pegada hídrica indireta**
associada ao consumo de energia elétrica em ambientes de HPC.

---

### 🇺🇸 English
This project implements an **energy simulation framework** for
High Performance Computing (HPC) platforms, focusing on the integration
of **photovoltaic (PV) solar energy** through the **PV-first energy policy**.

The PV-first policy prioritizes the use of locally generated photovoltaic
energy at each timestep, drawing electricity from the power grid only when
solar generation is insufficient to meet HPC energy demand.

The simulator was designed as the **primary artifact of a master’s research
project**, serving as the computational foundation for scientific analyses
related to energy sustainability and the **indirect water footprint**
associated with electricity consumption in HPC environments.

---

## 2. Contexto da Pesquisa | Research Context

### 🇧🇷 Português
Plataformas de HPC são amplamente utilizadas em pesquisas científicas,
indústria e aplicações governamentais, viabilizando simulações complexas
e processamento massivo de dados. Entretanto, essa capacidade computacional
está diretamente associada a **alto consumo energético contínuo**.

Grande parte da energia elétrica consumida por sistemas HPC é proveniente
de redes elétricas que utilizam, ao menos parcialmente, fontes termelétricas,
as quais demandam volumes significativos de água em seus processos de
resfriamento. Esse consumo indireto de água constitui a chamada
**pegada hídrica indireta da computação**, um aspecto ainda pouco explorado
na literatura de HPC.

A integração de energia fotovoltaica surge como uma alternativa promissora
para reduzir a dependência da rede elétrica e, consequentemente, o consumo
indireto de recursos hídricos.

---

### 🇺🇸 English
HPC platforms are widely used in scientific research, industry, and
governmental applications, enabling complex simulations and large-scale
data processing. However, this computational capability is directly
associated with **high and continuous energy consumption**.

A significant portion of the electricity consumed by HPC systems is supplied
by power grids that partially rely on thermoelectric generation, which
requires large volumes of water for cooling processes. This indirect water
use constitutes the **indirect water footprint of computing**, an aspect
still underexplored in HPC literature.

The integration of photovoltaic energy emerges as a promising alternative
to reduce grid dependency and, consequently, indirect water consumption.

---

## 3. Objetivo do Simulador | Simulator Objective

### 🇧🇷 Português
O objetivo principal deste simulador é **avaliar o impacto da política
energética PV-first** sobre:
- o consumo de energia da rede elétrica;
- o aproveitamento da energia fotovoltaica;
- a redução da pegada hídrica indireta associada ao consumo energético
  em sistemas HPC.

O simulador não busca reproduzir medições reais de hardware, mas sim
fornecer um **ambiente controlado e reprodutível** para análise de políticas
energéticas por meio de simulação computacional.

---

### 🇺🇸 English
The main objective of this simulator is to **evaluate the impact of the
PV-first energy policy** on:
- electrical grid energy consumption;
- utilization of photovoltaic energy;
- reduction of the indirect water footprint associated with energy
  consumption in HPC systems.

The simulator does not aim to reproduce real hardware measurements, but
rather to provide a **controlled and reproducible environment** for the
analysis of energy policies through computational simulation.

---

## 4. Escopo da Pesquisa | Research Scope

### 🇧🇷 Português
Incluído no escopo:
- Modelagem da geração de energia fotovoltaica;
- Modelagem do consumo energético de um nó HPC;
- Implementação da política energética PV-first;
- Cálculo de métricas energéticas e ambientais;
- Estimativa da pegada hídrica indireta evitada.

Fora do escopo:
- Modelagem de emissões de CO₂;
- Sistemas de armazenamento de energia (baterias);
- Escalonamento de tarefas ou workloads;
- Mecanismos de sistema operacional ou kernel;
- Medições em hardware real.

---

### 🇺🇸 English
Included in scope:
- Photovoltaic energy generation modeling;
- HPC node energy consumption modeling;
- Implementation of the PV-first energy policy;
- Computation of energy and environmental metrics;
- Estimation of avoided indirect water footprint.

Out of scope:
- Carbon emission modeling;
- Energy storage systems (batteries);
- Task or workload scheduling;
- Operating system or kernel-level mechanisms;
- Real hardware measurements.

---

## 5. Política Energética PV-first | PV-first Energy Policy

### 🇧🇷 Português
A política PV-first estabelece que toda energia fotovoltaica disponível
em um determinado intervalo de tempo deve ser utilizada prioritariamente.
Caso essa energia seja insuficiente para suprir a demanda do HPC, a rede
elétrica é utilizada apenas para complementar o déficit energético.

Formalmente:
- `E_PV_used(t) = min(E_PV_available(t), E_demand(t))`
- `E_grid(t) = E_demand(t) - E_PV_used(t)`

---

### 🇺🇸 English
The PV-first policy establishes that all photovoltaic energy available
at a given timestep must be used first. If this energy is insufficient
to meet HPC demand, the electrical grid is used only to supply the
remaining energy deficit.

Formally:
- `E_PV_used(t) = min(E_PV_available(t), E_demand(t))`
- `E_grid(t) = E_demand(t) - E_PV_used(t)`

---

## 6. Arquitetura do Simulador | Simulator Architecture

### 🇧🇷 Português
O simulador é organizado de forma modular, refletindo os conceitos
científicos da pesquisa:
- **Modelos energéticos**: geração fotovoltaica e consumo HPC;
- **Política energética**: decisão PV-first;
- **Métrica ambiental**: pegada hídrica indireta;
- **Motor de simulação**: execução temporal e agregação de resultados.

Cada módulo corresponde diretamente a um componente da metodologia
descrita na dissertação.

---

### 🇺🇸 English
The simulator is modularly organized, reflecting the scientific concepts
of the research:
- **Energy models**: photovoltaic generation and HPC consumption;
- **Energy policy**: PV-first decision logic;
- **Environmental metric**: indirect water footprint;
- **Simulation engine**: time-stepped execution and result aggregation.

Each module directly corresponds to a component of the methodology
described in the dissertation.

---

## 7. Estrutura do Projeto | Project Structure

```text
pv_first_hpc_simulator/
│
├── energy/
│   ├── pv_model.py        # Modelo fotovoltaico | PV model
│   └── hpc_model.py       # Modelo de consumo HPC | HPC consumption model
│
├── policy/
│   └── pv_first.py        # Política PV-first | PV-first policy
│
├── metrics/
│   └── water.py           # Pegada hídrica indireta | Indirect water footprint
│
├── simulation/
│   └── simulator.py       # Motor de simulação | Simulation engine
│
├── main.py                # Execução do experimento | Experiment entry point
├── requirements.txt       # Dependências | Dependencies
└── README.md              # Documentação | Documentation