# Documentazione del Componente Victron

## 📖 Introduzione

Il componente **Victron** è un'integrazione ESPHome che consente di comunicare con i dispositivi Victron Energy tramite protocollo UART. Supporta il controllo e il monitoraggio di sistemi ESS (Energy Storage System) e altri dispositivi compatibili.

## ⚠️ Prerequisiti Importanti

Prima di utilizzare questo componente, leggi attentamente:
- **[DISCLAIMER.md](./DISCLAIMER.md)** - Scarico di responsabilità legale
- Questa documentazione completa

## 🔧 Requisiti di Sistema

- **ESPHome**: Versione 2023.0 o superiore
- **Hardware**: Microcontroller con supporto UART (ESP32, ESP8266, ecc.)
- **Connessione seriale**: UART configurato correttamente per il dispositivo Victron
- **Librerie dipendenti**: ESPHome (esterne)

## 📦 Installazione

### 1. Configurazione nel progetto ESPHome

Aggiungi il componente esterno al tuo file `configuration.yaml`:

```yaml
external_components:
    - source: github://burlizzi/victron
      components: [ ess ]
      refresh: 300s
```

### 2. Configurazione dell'UART

Configura la connessione UART verso il dispositivo Victron:

```yaml
uart:
  id: uart_bus
  rx_pin: GPIO16          # Pin RX del tuo board
  tx_pin: GPIO17          # Pin TX del tuo board
  flow_control_pin: GPIO4 # Pin flow control (opzionale)
  baud_rate: 256000       # Velocità standard Victron
```

**Nota**: Verifica i pin corretti per il tuo microcontroller nella documentazione ESPHome.

### 3. Configurazione del Componente ESS

```yaml
ess:
  id: ess_unit
  uart_id: uart_bus
```

## 🎮 Controllo e Monitoraggio

### Switch per ON/OFF

Crea uno switch template per controllare il sistema:

```yaml
switch:
  - platform: template
    name: "ESS ON/OFF"
    optimistic: true
    turn_on_action:
      - lambda: |
          auto ess = id(ess_unit);
          ess->on();
    turn_off_action:
      - lambda: |
          auto ess = id(ess_unit);
          ess->off();
```

### Sensori (esempio)

Puoi esporre dati tramite sensori ESPHome (configura secondo le capacità del tuo dispositivo):

```yaml
sensor:
  - platform: template
    name: "Stato ESS"
    unit_of_measurement: ""
    lambda: return 0.0;  # Implementa logica di lettura
```

## 🔌 Connessioni Fisiche

### Schema di collegamento UART tipico

```
Victron Device          ESP Board
─────────────────────────────────────
GND          ────────► GND
RX (out)     ────────► GPIO16 (RX)
TX (in)      ────────► GPIO17 (TX)
Flow Control ────────► GPIO4 (opzionale)
```

**Importante**: 
- Verifica i livelli di tensione (alcuni dispositivi potrebbero richiedere resistori di pull-up)
- Utilizza cavi schermati per ridurre l'interferenza
- Mantieni i cavi il più corti possibile

## 🐛 Troubleshooting

### Problema: Nessuna comunicazione con il dispositivo

**Soluzioni**:
1. Verifica i pin UART configurati
2. Controlla la velocità in baud (dovrebbe essere 256000)
3. Verifica il collegamento fisico e i cavi
4. Controlla i livelli di tensione

### Problema: Errori di comunicazione

**Soluzioni**:
1. Riduci il numero di periferiche UART simultanee
2. Aggiungi resistori di pull-up (4.7kΩ tipicamente)
3. Accorcia i cavi UART
4. Verifica l'alimentazione del dispositivo Victron

### Problema: Dispositivo non risponde ai comandi

**Soluzioni**:
1. Verifica che il dispositivo sia acceso e connesso
2. Controlla lo stato nel log di ESPHome
3. Prova a riprogrammare il microcontroller
4. Verifica la versione del firmware del dispositivo Victron

## 📊 Configurazione Completa (Esempio)

```yaml
external_components:
    - source: github://burlizzi/victron
      components: [ ess ]
      refresh: 300s

uart:
  id: uart_bus
  rx_pin: GPIO16
  tx_pin: GPIO17
  flow_control_pin: GPIO4
  baud_rate: 256000

ess:
  id: ess_unit
  uart_id: uart_bus

switch:
  - platform: template
    name: "ESS ON/OFF"
    optimistic: true
    turn_on_action:
      - lambda: |
          auto ess = id(ess_unit);
          ess->on();
    turn_off_action:
      - lambda: |
          auto ess = id(ess_unit);
          ess->off();
```

## 🔐 Note di Sicurezza

- **Non esporre** il componente su reti pubbliche senza autenticazione
- **Implementa** protezioni hardware appropriate se usato in sistemi critici
- **Testa** sempre in ambiente controllato prima di deployment
- **Monitora** costantemente in produzione

## 📞 Supporto e Segnalazione Bug

- **GitHub Issues**: Segnala bug e problemi
- **Documentazione ESPHome**: https://esphome.io/
- **Documentazione Victron**: https://www.victronenergy.com/

## 📝 Licenza

Consulta il file LICENSE nel repository per i dettagli sulla licenza.

## ⚖️ Disclaimer Legale

Questo software è fornito **SENZA GARANZIA**. Leggi il [DISCLAIMER.md](./DISCLAIMER.md) prima di utilizzarlo, soprattutto se prevedi di usarlo in ambienti critici.

---

**Versione Documentazione**: 1.0  
**Ultima modifica**: 2026-05-11  
**Autore**: Burlizzi
