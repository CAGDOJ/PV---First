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
    std::cout << "1) o SimGrid executa o job e mede a energia exigida pelo host\n";
    std::cout << "2) eu calculo a energia solar disponivel com base no local e no clima\n";
    std::cout << "3) a politica PV-First tenta atender primeiro o job com a placa\n\n";

    double jobFlops = askJobFlops();

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

    if (irradianceTheoreticalWm2 <= 0.0) {
        std::cout << "Observacao: neste horario a irradiancia teorica ficou zerada.\n";
        std::cout << "Isso normalmente acontece porque o calculo caiu em periodo noturno ou muito proximo disso.\n";
    }

    // ============================= JOB DO SIMGRID ============================
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
    // Exemplo:
    // results/RPVfirst180426.csv
    //
    // Se eu rodar varias vezes no mesmo dia, ele continua acrescentando linhas nesse mesmo arquivo.
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

    // Aqui eu deixei o cabecalho pronto para Tableau e banco:
    // nomes estaveis, sem espaco e com unidade no nome.
    if (!fileExists) {
        file << "run_id,"
                "run_date,"
                "run_time,"
                "run_datetime,"
                "day_of_year,"
                "city,"
                "latitude,"
                "longitude,"
                "panel_material,"
                "panel_face_type,"
                "panel_area_m2,"
                "panel_base_efficiency,"
                "panel_material_factor,"
                "panel_effective_base_efficiency,"
                "panel_bifacial_gain_factor,"
                "cloud_cover_pct,"
                "rain_mm,"
                "temperature_c,"
                "wind_speed_kmh,"
                "irradiance_theoretical_w_m2,"
                "irradiance_adjusted_w_m2,"
                "pv_efficiency,"
                "pv_power_kw,"
                "grid_carbon_intensity_gco2_kwh,"
                "job_flops,"
                "job_duration_s,"
                "job_energy_j,"
                "job_energy_kwh,"
                "job_average_power_kw,"
                "energy_total_kwh,"
                "energy_pv_kwh,"
                "energy_grid_kwh,"
                "co2_g\n";
    }

    file << runIdBuilder.str() << ","
         << runDateBuilder.str() << ","
         << runTimeBuilder.str() << ","
         << runDateTimeBuilder.str() << ","
         << dayOfYear << ","
         << "\"" << gps.city << "\"" << ","
         << gps.latitude << ","
         << gps.longitude << ","
         << "\"" << config.pv.panelMaterial << "\"" << ","
         << "\"" << config.pv.panelFaceType << "\"" << ","
         << config.pv.panelAreaM2 << ","
         << config.pv.baseEfficiency << ","
         << materialFactor << ","
         << effectiveBaseEfficiency << ","
         << config.pv.bifacialGainFactor << ","
         << impact.cloudCover << ","
         << impact.rainAmount << ","
         << impact.temperature << ","
         << impact.windSpeed << ","
         << irradianceTheoreticalWm2 << ","
         << irradianceAdjustedWm2 << ","
         << pvEfficiency << ","
         << pvPowerKW << ","
         << config.gridCarbonIntensity << ","
         << job.jobFlops << ","
         << job.durationSeconds << ","
         << job.energyJoules << ","
         << job.energyKWh << ","
         << job.averagePowerKW << ","
         << stats.E_total << ","
         << stats.E_pv << ","
         << stats.E_grid << ","
         << stats.CO2 << "\n";

    file.close();

    std::cout << "\nDados salvos em: "
              << resultsFilePath.string() << "\n";

    std::cout << "\n============================================================\n";
    std::cout << "SIMULACAO FINALIZADA\n";
    std::cout << "============================================================\n";
}