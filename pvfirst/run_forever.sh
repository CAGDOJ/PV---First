#!/bin/bash
set -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT_DIR"

LOG_FILE="$ROOT_DIR/daemon.log"

echo "==================================================" | tee -a "$LOG_FILE"
echo "Inicio do modo continuo: $(date '+%Y-%m-%d %H:%M:%S')" | tee -a "$LOG_FILE"
echo "==================================================" | tee -a "$LOG_FILE"

while true; do
    NOW="$(date '+%Y-%m-%d %H:%M:%S')"

    echo "[$NOW] Iniciando nova execucao..." | tee -a "$LOG_FILE"

    if printf '\n' | ./run.sh >> "$LOG_FILE" 2>&1; then
        echo "[$NOW] Execucao finalizada com sucesso." | tee -a "$LOG_FILE"
    else
        echo "[$NOW] A execucao falhou. Vou tentar novamente em 15 segundos." | tee -a "$LOG_FILE"
        sleep 15
        continue
    fi

    echo "[$NOW] Aguardando 60 segundos para a proxima execucao." | tee -a "$LOG_FILE"
    sleep 60
done