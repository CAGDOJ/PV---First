#!/bin/bash

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT_DIR" || exit 1

LOG_FILE="$ROOT_DIR/solar_window.log"

NO_IRRADIANCE_AFTER_NOON=0
LIMIT_NO_IRRADIANCE_AFTER_NOON=5

DAY_KEY="$(date '+%Y-%m-%d')"
DAY_FINISHED=0

echo "==================================================" | tee -a "$LOG_FILE"
echo "Modo solar continuo iniciado em: $(date '+%Y-%m-%d %H:%M:%S')" | tee -a "$LOG_FILE"
echo "O sistema observa a irradiancia a partir das 05:00." | tee -a "$LOG_FILE"
echo "Quando a irradiancia acabar, entra em standby noturno." | tee -a "$LOG_FILE"
echo "No dia seguinte, volta automaticamente a observar." | tee -a "$LOG_FILE"
echo "==================================================" | tee -a "$LOG_FILE"

while true; do
    NOW="$(date '+%Y-%m-%d %H:%M:%S')"
    CURRENT_DAY="$(date '+%Y-%m-%d')"
    HOUR="$(date +%H)"
    HOUR_NUM=$((10#$HOUR))

    # Se virou o dia, reseta o controle.
    if [ "$CURRENT_DAY" != "$DAY_KEY" ]; then
        DAY_KEY="$CURRENT_DAY"
        DAY_FINISHED=0
        NO_IRRADIANCE_AFTER_NOON=0

        echo "==================================================" | tee -a "$LOG_FILE"
        echo "[$NOW] Novo dia detectado. Saindo do standby e reiniciando observacao solar." | tee -a "$LOG_FILE"
        echo "==================================================" | tee -a "$LOG_FILE"
    fi

    # Antes das 05:00 fica em standby noturno.
    if [ "$HOUR_NUM" -lt 5 ]; then
        echo "[$NOW] STANDBY NOTURNO: aguardando 05:00 para iniciar a observacao solar." | tee -a "$LOG_FILE"
        sleep 300
        continue
    fi

    # Se o dia solar ja terminou, fica em standby ate virar o dia.
    if [ "$DAY_FINISHED" -eq 1 ]; then
        echo "[$NOW] STANDBY NOTURNO: experimento de hoje finalizado. Aguardando o proximo dia." | tee -a "$LOG_FILE"
        sleep 300
        continue
    fi

    TMP_OUTPUT="$(mktemp)"

    echo "[$NOW] Verificando irradiancia e executando simulacao..." | tee -a "$LOG_FILE"

    printf '\n' | ./run.sh > "$TMP_OUTPUT" 2>&1
    RUN_STATUS=$?

    cat "$TMP_OUTPUT" >> "$LOG_FILE"

    if [ "$RUN_STATUS" -ne 0 ]; then
        echo "[$NOW] A execucao falhou. Vou tentar novamente em 60 segundos." | tee -a "$LOG_FILE"
        rm -f "$TMP_OUTPUT"
        sleep 60
        continue
    fi

    if grep -q "PVFIRST_SEM_IRRADIANCIA" "$TMP_OUTPUT"; then
        if [ "$HOUR_NUM" -lt 12 ]; then
            echo "[$NOW] Ainda sem irradiancia, mas pode ser antes do nascer efetivo do sol. Continuando..." | tee -a "$LOG_FILE"
            NO_IRRADIANCE_AFTER_NOON=0
        else
            NO_IRRADIANCE_AFTER_NOON=$((NO_IRRADIANCE_AFTER_NOON + 1))

            echo "[$NOW] Sem irradiancia apos o meio-dia. Contador: $NO_IRRADIANCE_AFTER_NOON/$LIMIT_NO_IRRADIANCE_AFTER_NOON" | tee -a "$LOG_FILE"

            if [ "$NO_IRRADIANCE_AFTER_NOON" -ge "$LIMIT_NO_IRRADIANCE_AFTER_NOON" ]; then
                echo "[$NOW] Irradiancia encerrada por varios minutos." | tee -a "$LOG_FILE"
                echo "[$NOW] Entrando em STANDBY NOTURNO. O programa continua aberto." | tee -a "$LOG_FILE"

                DAY_FINISHED=1
                NO_IRRADIANCE_AFTER_NOON=0
            fi
        fi
    else
        echo "[$NOW] Irradiancia util detectada. Resultado salvo normalmente." | tee -a "$LOG_FILE"
        NO_IRRADIANCE_AFTER_NOON=0
    fi

    rm -f "$TMP_OUTPUT"

    sleep 60
done