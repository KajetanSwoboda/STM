import serial
import json
import keyboard
import time
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider

# Konfiguracja portu szeregowego
hSerial = serial.Serial('COM7', 115200, timeout=1, parity=serial.PARITY_NONE)
rpm = 1000  # Zmienna do przechowywania wartości suwaka

# Konfiguracja pliku wyjściowego
timestr = time.strftime("%H_%M_%S_%d_%m_%Y")  # GodzinaMinutaSekunda_DzieńMiesiącRok
hFile = open(f"Zebrane_dane_z_{timestr}.txt", "a")
# Dane do wykresu
timestamps = []
rpm_values = []
set_rpm_values = []
duty_values = []

# Tryb trackbar
trackbar_mode = False

# Funkcja obsługująca zmianę suwaka RPM
def on_slider_change(value):
    global rpm
    rpm = int(value)
    mode_value = "1" if trackbar_mode else "0"  # "1" jeśli tryb trackbar aktywny, "0" w przeciwnym razie
    send_data = f"{rpm:04}{mode_value}"  # Formatujemy dane: 4 cyfry RPM i 1 cyfra trybu
    hSerial.write(send_data.encode('utf-8'))  # Wysyłamy zaktualizowaną wartość przez port szeregowy
    print(f"Sent {send_data}")

# Funkcja obsługująca zmianę trybu trackbar
def on_trackbar_change(value):
    global trackbar_mode
    trackbar_mode = value > 0.5  # Tryb trackbar, jeśli wartość suwaka jest większa niż 0.5
    print(f"Trackbar mode: {trackbar_mode}")
    # Po zmianie trybu także wysyłamy dane
    mode_value = "1" if trackbar_mode else "0"
    send_data = f"{rpm:04}{mode_value}"
    hSerial.write(send_data.encode('utf-8'))  # Wysyłamy zaktualizowaną wartość przez port szeregowy
    print(f"Sent {send_data}")

# Główna funkcja programu
def main():
    global timestamps, rpm_values, set_rpm_values, duty_values

    # Tworzenie okna wykresu Matplotlib
    fig, ax = plt.subplots(figsize=(10, 6))
    plt.subplots_adjust(left=0.1, bottom=0.4)

    # Wykres początkowy
    l, = plt.plot([], [], label="RPM", color="blue")
    l_set, = plt.plot([], [], label="Set RPM (RPM_REF)", color="red")
    plt.xlabel("Czas (s)")
    plt.ylabel("RPM")
    plt.title("Wykres RPM na żywo")
    plt.legend()
    plt.grid()

    # Suwak do ustawienia RPM
    ax_slider = plt.axes([0.1, 0.25, 0.8, 0.05], facecolor='lightgoldenrodyellow')
    slider = Slider(ax_slider, 'RPM', 1000, 3000, valinit=1000, valstep=1)
    slider.on_changed(on_slider_change)

    # Suwak do przełączania trybu trackbar
    ax_trackbar = plt.axes([0.1, 0.15, 0.8, 0.05], facecolor='lightgoldenrodyellow')
    trackbar = Slider(ax_trackbar, 'Mode', 0, 1, valinit=0, valstep=1)
    trackbar.on_changed(on_trackbar_change)

    # Pole tekstowe do wyświetlania błędu
    error_text = fig.text(0.5, 0.1, "e: 0.00%", ha="center", fontsize=12, color="black")

    start_time = time.time()

    while True:
        if keyboard.is_pressed("q"):
            break

        # Odbieranie danych z portu szeregowego
        time.sleep(0.1)
        received_data = hSerial.readline().decode("utf-8").strip()
        if received_data:
            print(f"Received data: {received_data}")

            try:
                sample = json.loads(received_data)
            except ValueError:
                print("Bad JSON")
                print(f"{received_data}\r")
                continue

            # Zbieranie danych do wykresu
            current_time = time.time() - start_time
            timestamps.append(current_time)
            received_rpm = sample.get("RPM", 0)
            received_duty = sample.get("Duty", 0)
            rpm_ref = sample.get("RPM_REF", 0)

            rpm_values.append(received_rpm)
            duty_values.append(received_duty)
            set_rpm_values.append(rpm_ref)

            if len(timestamps) > 100:
                timestamps = timestamps[-100:]
                rpm_values = rpm_values[-100:]
                set_rpm_values = set_rpm_values[-100:]
                duty_values = duty_values[-100:]

            # Zapisywanie danych w formacie JSON
            log_entry = {
                "RPM": received_rpm,
                "RPM_REF": rpm_ref,
                "Duty": received_duty
            }
            hFile.write(json.dumps(log_entry) + "\n")
            hFile.flush()

            # Obliczanie błędu regulacji
            if len(rpm_values) > 0 and len(set_rpm_values) > 0:
                current_rpm = rpm_values[-1]
                set_rpm = set_rpm_values[-1]
                error_percentage = (abs(set_rpm - current_rpm) / set_rpm) * 100 if set_rpm != 0 else 0
                error_text.set_text(
                    f"e: {error_percentage:.2f}% | RPM zadane: {set_rpm:.0f} | RPM aktualne: {current_rpm:.0f}"
                )

            # Aktualizacja wykresu
            l.set_data(timestamps, rpm_values)
            l_set.set_data(timestamps, set_rpm_values)

            ax.relim()
            ax.autoscale_view()

            plt.draw()
            plt.pause(0.1)

    plt.close(fig)
    hFile.close()

# Uruchomienie programu
main()
hSerial.close()
