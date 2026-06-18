#include "SimulationController.hpp"
#include "SimGridJobRunner.hpp"
#include "sensors/GeoSensor.hpp"
#include "sensors/MetarSensor.hpp"
#include "sensors/SolarModel.hpp"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{
    // Aqui eu converti o tipo de material em um fator multiplicador simples.
    // A base do projeto continua sendo a eficiencia configurada em baseEfficiency.
    // O material ajusta essa base para representar paineis diferentes sem complicar demais o modelo.
    double getPanelMaterialFactor(const std::string& material)
    {
        if (material == "monocrystalline")
            return 1.00;

        if (material == "polycrystalline")
            return 0.90;

        if (material == "thinfilm")
            return 0.65;

        // Se vier um texto inesperado, eu nao travo o programa.
        // So volto para um fator neutro.
        return 1.00;
    }

    // Aqui eu trato o efeito de monofacial ou bifacial.
    // Se for bifacial, eu aplico um ganho extra configuravel.
    double getPanelFaceGain(const std::string& faceType, double bifacialGainFactor)
    {
        if (faceType == "bifacial")
            return bifacialGainFactor;

        return 1.0;
    }
}

SimulationController::SimulationController()
    : config(),
      model(config.gridCarbonIntensity)
{
}

double SimulationController::parseJobInput(const std::string& input)
{
    // Se eu apertar Enter sem digitar nada, uso o valor padrao configurado.
    if (input.empty())
        return config.defaultJobFlops;

    return std::stod(input);
}

double SimulationController::askJobFlops()
{
    std::string input;

    std::cout << "Digite a carga do job em FLOPs (ex: 5e10).\n";
    std::cout << "Se quiser usar o valor padrao, e so apertar Enter: ";
    std::getline(std::cin, input);

    return parseJobInput(input);
}

void SimulationController::run()
{
    std::cout << "\n============================================================\n";
    std::cout << "SIMULACAO PV-FIRST COM JOB DO SIMGRID\n";
    std::cout << "============================================================\n\n";

    std::cout << "Fluxo da simulacao:\n";
    std::cout << "1) eu verifico local, clima e irradiancia solar\n";
    std::cout << "2) se houver irradiancia util, o SimGrid executa o job\n";
    std::cout << "3) a politica PV-First tenta atender primeiro o job com a placa\n\n";

    // ============================== LOCALIZACAO ==============================
    // Aqui eu pego a localizacao atual do experimento.
    // Isso serve de base para o clima e para o calculo solar.
    GeoSensor geo;
    GPSData gps = geo.getLocation();

    // ========================== HORA LOCAL E DIA =============================
    // Aqui eu uso a hora local da maquina.
    // Isso define o dia do ano e a hora decimal que entram no modelo solar.
    std::time_t now = std::time(nullptr);
    std::tm localTime = *std::localtime(&now);

    int dayOfYear = localTime.tm_yday + 1;
    int hourInt   = localTime.tm_hour;
    int minuteInt = localTime.tm_min;
    int secondInt = localTime.tm_sec;

    double hourDecimal = hourInt + minuteInt / 60.0;

    // ========================== CLIMA E IRRADIANCIA ==========================
    // Primeiro eu calculo a irradiancia teorica.
    // Depois puxo os fatores meteorologicos reais.
    SolarModel solar;
    double irradianceTheoreticalWm2 =
        solar.computeIrradiance(gps.latitude, dayOfYear, hourDecimal);

    MetarSensor metar;
    WeatherImpact impact = metar.getWeatherImpact(gps.latitude, gps.longitude);

    // Aqui eu reduzo a irradiancia teorica com os fatores de nuvem e chuva.
    double irradianceAdjustedWm2 =
        irradianceTheoreticalWm2 *
        impact.cloudFactor *
        impact.rainFactor;

    // ======================== PARAMETROS DO PAINEL ===========================
    // Aqui eu aplico a parte modular do painel sem criar arquivo novo.
    // A ideia e:
    // - baseEfficiency representa a eficiencia de referencia do experimento
    // - panelMaterial ajusta essa base para o tipo de tecnologia
    // - panelFaceType ajusta o ganho extra se o painel for bifacial
    double materialFactor =
        getPanelMaterialFactor(config.pv.panelMaterial);

    double effectiveBaseEfficiency =
        config.pv.baseEfficiency * materialFactor;

    double faceGain =
        getPanelFaceGain(
            config.pv.panelFaceType,
            config.pv.bifacialGainFactor
        );

    // Agora eu monto a eficiencia final do arranjo.
    // Primeiro ajusto pela tecnologia do painel.
    // Depois aplico temperatura, vento e eventualmente ganho bifacial.
    double pvEfficiency =
        effectiveBaseEfficiency *
        impact.tempFactor *
        impact.windCoolingFactor *
        faceGain;

    double pvPowerKW =
        pvEfficiency *
        config.pv.panelAreaM2 *
        irradianceAdjustedWm2 / 1000.0;

    if (pvPowerKW < 0.0)
        pvPowerKW = 0.0;

    std::cout << "\n-------------------- DADOS DO LOCAL --------------------\n";
    std::cout << "Cidade detectada : " << gps.city << "\n";
    std::cout << "Latitude         : " << gps.latitude << "\n";
    std::cout << "Longitude        : " << gps.longitude << "\n";
    std::cout << "Hora local       : "
              << std::setfill('0') << std::setw(2) << hourInt << ":"
              << std::setfill('0') << std::setw(2) << minuteInt << ":"
              << std::setfill('0') << std::setw(2) << secondInt << "\n";
    std::cout << "Dia do ano       : " << dayOfYear << "\n";

    std::cout << "\n------------------ CONDICOES DO CLIMA ------------------\n";
    std::cout << "Cobertura nuvens : " << impact.cloudCover << " %\n";
    std::cout << "Chuva            : " << impact.rainAmount << " mm\n";
    std::cout << "Temperatura      : " << impact.temperature << " C\n";
    std::cout << "Vento            : " << impact.windSpeed << " km/h\n";

    std::cout << "\n----------------- CONFIGURACAO DO PAINEL ----------------\n";
    std::cout << "Material          : " << config.pv.panelMaterial << "\n";
    std::cout << "Face do painel    : " << config.pv.panelFaceType << "\n";
    std::cout << "Area do painel    : " << config.pv.panelAreaM2 << " m2\n";
    std::cout << "Eficiencia base   : " << config.pv.baseEfficiency << "\n";
    std::cout << "Ganho bifacial    : " << config.pv.bifacialGainFactor << "\n";
    std::cout << "Fator do material : " << materialFactor << "\n";

    std::cout << "\n----------------- MODELO FOTOVOLTAICO ------------------\n";
    std::cout << "Irradiancia teorica      : " << irradianceTheoreticalWm2 << " W/m2\n";
    std::cout << "Irradiancia ajustada     : " << irradianceAdjustedWm2 << " W/m2\n";
    std::cout << "Eficiencia base efetiva  : " << effectiveBaseEfficiency << "\n";
    std::cout << "Eficiencia final arranjo : " << pvEfficiency << "\n";
    std::cout << "Potencia PV disponivel   : " << pvPowerKW << " kW\n";

    // ====================== FILTRO DE IRRADIANCIA UTIL =======================
    // Aqui esta o ponto principal para nao encher o CSV com dados sem sentido.
    //
    // Se ainda nao tiver sol util, eu paro esta execucao aqui.
    // Assim:
    // - nao rodo o job no SimGrid sem necessidade
    // - nao salvo linha no CSV
    // - o script externo entende que ainda nao comecou o periodo solar
    //   ou que o periodo solar ja terminou.
    //
    // Esse texto PVFIRST_SEM_IRRADIANCIA e proposital.
    // O script run_solar_window.sh usa exatamente essa frase para decidir
    // se continua aguardando ou se encerra o experimento do dia.
    const double irradianceMinimumToRun = 1.0; // W/m2

    if (irradianceAdjustedWm2 <= irradianceMinimumToRun || pvPowerKW <= 0.0) {
        std::cout << "\nPVFIRST_SEM_IRRADIANCIA\n";
        std::cout << "Sem irradiancia util neste instante.\n";
        std::cout << "Nenhum job foi executado no SimGrid.\n";
        std::cout << "Nenhum resultado foi salvo no CSV.\n";
        std::cout << "\n============================================================\n";
        std::cout << "EXECUCAO ENCERRADA SEM REGISTRO\n";
        std::cout << "============================================================\n";
        return;
    }

    // ============================= JOB DO SIMGRID ============================
    // A partir daqui eu ja sei que existe irradiancia util.
    // Entao agora sim vale a pena executar o job no SimGrid e registrar o resultado.
    double jobFlops = askJobFlops();

    // Aqui o SimGrid continua sendo a fonte oficial da demanda do job.
    // Ou seja: a duracao, a energia e a potencia media saem da simulacao computacional,
    // e nao de um chute feito no controller.
    SimGridJobRunner jobRunner;
    SimGridJobConfig jobConfig;
    jobConfig.jobFlops = jobFlops;

    SimGridJobResult job = jobRunner.run(jobConfig);

    std::cout << "\n--------------------- JOB DO SIMGRID -------------------\n";
    std::cout << "Host usado           : " << job.hostName << "\n";
    std::cout << "Carga do job         : " << job.jobFlops << " FLOPs\n";
    std::cout << "Velocidade do host   : " << job.hostSpeedFlops << " flop/s\n";
    std::cout << "Duracao do job       : " << job.durationSeconds << " s\n";
    std::cout << "Energia do job       : " << job.energyJoules << " J\n";
    std::cout << "Energia do job       : " << job.energyKWh << " kWh\n";
    std::cout << "Potencia media do job: " << job.averagePowerKW << " kW\n";

    // ============================= TRIAGEM PV-FIRST ==========================
    // Aqui eu junto os dois lados do problema:
    // - o SimGrid me diz quanto o job exigiu
    // - o modelo solar me diz quanto a PV consegue fornecer
    //
    // A politica PV-First entra justamente aqui:
    // primeiro tenta atender com a placa, depois empurra o resto para a rede.
    model.update(job.averagePowerKW, pvPowerKW, job.durationSeconds);
    EnergyStats stats = model.getStats();

    double pvPossibleKWh = pvPowerKW * (job.durationSeconds / 3600.0);

    std::cout << "\n-------------------- RESULTADO PV-FIRST ----------------\n";
    std::cout << "Energia total do job  : " << stats.E_total << " kWh\n";
    std::cout << "Energia vinda da PV   : " << stats.E_pv << " kWh\n";
    std::cout << "Energia vinda da rede : " << stats.E_grid << " kWh\n";
    std::cout << "CO2 da parte da rede  : " << stats.CO2 << " gCO2\n";

    std::cout << "\nLeitura rapida do experimento:\n";
    std::cout << "- o job do SimGrid pediu " << job.energyKWh << " kWh no total\n";
    std::cout << "- a placa poderia entregar ate " << pvPossibleKWh << " kWh nesse mesmo intervalo\n";
    std::cout << "- a politica PV-First usou primeiro a energia solar e mandou o resto para a rede\n";

        // ================================ CSV ====================================
    // Aqui eu salvo um arquivo por dia dentro da pasta results na raiz do projeto.
    // Agora usei ponto e virgula como separador, porque no Excel em portugues
    // o CSV com virgula costuma abrir todo baguncado.
    //
    // Exemplo:
    // results/RPVfirst170626.csv

    std::filesystem::create_directories("results");

    std::ostringstream fileNameBuilder;
    fileNameBuilder << "RPVfirst"
                    << std::setfill('0') << std::setw(2) << localTime.tm_mday
                    << std::setfill('0') << std::setw(2) << (localTime.tm_mon + 1)
                    << std::setfill('0') << std::setw(2) << ((localTime.tm_year + 1900) % 100)
                    << ".csv";

    std::filesystem::path resultsFilePath =
        std::filesystem::path("results") / fileNameBuilder.str();

    bool fileExists = std::filesystem::exists(resultsFilePath);

    std::ofstream file(resultsFilePath, std::ios::app);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Nao consegui abrir ou criar o arquivo de resultados em: " +
            resultsFilePath.string()
        );
    }

    std::ostringstream runDateBuilder;
    runDateBuilder << (localTime.tm_year + 1900) << "-"
                   << std::setfill('0') << std::setw(2) << (localTime.tm_mon + 1) << "-"
                   << std::setfill('0') << std::setw(2) << localTime.tm_mday;

    std::ostringstream runTimeBuilder;
    runTimeBuilder << std::setfill('0') << std::setw(2) << hourInt << ":"
                   << std::setfill('0') << std::setw(2) << minuteInt << ":"
                   << std::setfill('0') << std::setw(2) << secondInt;

    std::ostringstream runDateTimeBuilder;
    runDateTimeBuilder << runDateBuilder.str() << " " << runTimeBuilder.str();

    std::ostringstream runIdBuilder;
    runIdBuilder << "run_"
                 << (localTime.tm_year + 1900)
                 << std::setfill('0') << std::setw(2) << (localTime.tm_mon + 1)
                 << std::setfill('0') << std::setw(2) << localTime.tm_mday
                 << "_"
                 << std::setfill('0') << std::setw(2) << hourInt
                 << std::setfill('0') << std::setw(2) << minuteInt
                 << std::setfill('0') << std::setw(2) << secondInt;

    const char sep = ';';

    if (!fileExists) {
        file << "run_id" << sep
             << "run_date" << sep
             << "run_time" << sep
             << "run_datetime" << sep
             << "day_of_year" << sep
             << "city" << sep
             << "latitude" << sep
             << "longitude" << sep
             << "panel_material" << sep
             << "panel_face_type" << sep
             << "panel_area_m2" << sep
             << "panel_base_efficiency" << sep
             << "panel_material_factor" << sep
             << "panel_effective_base_efficiency" << sep
             << "panel_bifacial_gain_factor" << sep
             << "cloud_cover_pct" << sep
             << "rain_mm" << sep
             << "temperature_c" << sep
             << "wind_speed_kmh" << sep
             << "irradiance_theoretical_w_m2" << sep
             << "irradiance_adjusted_w_m2" << sep
             << "pv_efficiency" << sep
             << "pv_power_kw" << sep
             << "grid_carbon_intensity_gco2_kwh" << sep
             << "job_flops" << sep
             << "job_duration_s" << sep
             << "job_energy_j" << sep
             << "job_energy_kwh" << sep
             << "job_average_power_kw" << sep
             << "energy_total_kwh" << sep
             << "energy_pv_kwh" << sep
             << "energy_grid_kwh" << sep
             << "co2_g\n";
    }

    file << runIdBuilder.str() << sep
         << runDateBuilder.str() << sep
         << runTimeBuilder.str() << sep
         << runDateTimeBuilder.str() << sep
         << dayOfYear << sep
         << "\"" << gps.city << "\"" << sep
         << gps.latitude << sep
         << gps.longitude << sep
         << "\"" << config.pv.panelMaterial << "\"" << sep
         << "\"" << config.pv.panelFaceType << "\"" << sep
         << config.pv.panelAreaM2 << sep
         << config.pv.baseEfficiency << sep
         << materialFactor << sep
         << effectiveBaseEfficiency << sep
         << config.pv.bifacialGainFactor << sep
         << impact.cloudCover << sep
         << impact.rainAmount << sep
         << impact.temperature << sep
         << impact.windSpeed << sep
         << irradianceTheoreticalWm2 << sep
         << irradianceAdjustedWm2 << sep
         << pvEfficiency << sep
         << pvPowerKW << sep
         << config.gridCarbonIntensity << sep
         << job.jobFlops << sep
         << job.durationSeconds << sep
         << job.energyJoules << sep
         << job.energyKWh << sep
         << job.averagePowerKW << sep
         << stats.E_total << sep
         << stats.E_pv << sep
         << stats.E_grid << sep
         << stats.CO2 << "\n";

    file.close();

    std::cout << "\nDados salvos em: "
              << resultsFilePath.string() << "\n";

    std::cout << "\n============================================================\n";
    std::cout << "SIMULACAO FINALIZADA\n";
    std::cout << "============================================================\n";
}