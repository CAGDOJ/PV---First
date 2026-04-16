#include "SimGridJobRunner.hpp"

#include <simgrid/plugins/energy.h>
#include <simgrid/s4u.hpp>

#include <stdexcept>

namespace sg4 = simgrid::s4u;

SimGridJobResult SimGridJobRunner::run(const SimGridJobConfig& config)
{
    // Eu deixei essa parte isolada para a logica principal do projeto continuar limpa.
    // O papel daqui e so este:
    // 1) carregar a plataforma do SimGrid
    // 2) mandar um host executar um job com certa quantidade de FLOPs
    // 3) medir quanto tempo esse job levou
    // 4) medir quanta energia o host consumiu nesse intervalo

    int argc = 1;
    char programName[] = "pvfirst";
    char* argv[] = {programName, nullptr};

    sg4::Engine engine(&argc, argv);

    // O plugin de energia precisa ser ligado antes do load_platform.
    // Sem isso eu consigo simular o job, mas nao consigo ler o consumo energetico do host.
    sg_host_energy_plugin_init();

    try {
        engine.load_platform(config.platformPath);
    }
    catch (const std::exception&) {
        throw std::runtime_error(
            "Nao consegui abrir a plataforma do SimGrid em: " + config.platformPath +
            ". Verifique se o arquivo simgrid/platform.xml existe na raiz do projeto "
            "e se o CMake copiou esse arquivo para build/simgrid."
        );
    }

    sg4::Host* host = engine.host_by_name(config.hostName);
    if (host == nullptr) {
        throw std::runtime_error(
            "Nao encontrei o host '" + config.hostName +
            "' dentro da plataforma do SimGrid."
        );
    }

    double startTime   = 0.0;
    double finishTime  = 0.0;
    double energyStart = sg_host_get_consumed_energy(host);

    // Aqui nasce o job do SimGrid.
    // Eu crio um ator no host escolhido e mando esse ator executar a carga computacional.
    // O SimGrid converte essa carga em tempo de execucao de acordo com a velocidade do host.
    sg4::Actor::create("pvfirst_job", host, [config, &startTime, &finishTime]() {
        startTime = sg4::Engine::get_clock();

        // Essa linha representa o trabalho computacional do job.
        // Se eu aumentar jobFlops, o job passa a exigir mais tempo e mais energia do host.
        sg4::this_actor::execute(config.jobFlops);

        finishTime = sg4::Engine::get_clock();
    });

    engine.run();

    double energyFinish = sg_host_get_consumed_energy(host);

    SimGridJobResult result;
    result.hostName        = config.hostName;
    result.jobFlops        = config.jobFlops;
    result.durationSeconds = finishTime - startTime;
    result.energyJoules    = energyFinish - energyStart;
    result.energyKWh       = result.energyJoules / 3600000.0;
    result.hostSpeedFlops  = host->get_speed();

    if (result.durationSeconds > 0.0) {
        result.averagePowerKW = (result.energyJoules / result.durationSeconds) / 1000.0;
    }
    else {
        result.averagePowerKW = 0.0;
    }

    return result;
}