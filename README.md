![NOW: Time Header](https://github.com/atomlabor/Now-Time/raw/main/NOW%20Time%20BG.png)

# NOW: Time - Retro MP3 Watchface for Pebble

Turn your Pebble into a classic digital music player. **NOW: Time** combines pure early-2000s nostalgia with modern smartwatch features, bringing your current soundtrack and daily stats straight to your wrist. 

## ✨ Features

* **Live Last.fm Sync:** Connect your Last.fm account to display exactly what you are listening to right now. Long track names scroll smoothly across the screen in a retro marquee style so you never miss a beat.
* **Shake to Play:** Flick your wrist to switch from the standard time and date view to the music player interface. The display automatically switches back to the date after 12 seconds to save battery.
* **Classic Retro UI:** Inspired by legendary MP3 players, featuring a clean clickwheel design and a subtle battery level indicator built right into the playbar.
* **Health Integration:** Keep an eye on your daily steps and current heart rate directly on the main screen. 
* **Deep Customization:** Use the settings menu to pick your favorite header color, set a "Fallback Song" for when your music is paused, and toggle the vibration alerts for new tracks.

## ⚙️ Setup & Configuration

To get the most out of the live music features, you need to configure the watchface via the Pebble app settings (powered by Clay):

1. **Last.fm Username:** Enter your Last.fm username in the settings. 
2. **Public Profile:** Ensure your Last.fm profile is set to *public* so the watch can fetch your current tracks.
3. **Polling:** The watch automatically and reliably checks for a new song every 20 seconds.
4. **Vibration:** You can enable or disable the short vibration pulse that triggers when a new song starts playing.

## 📱 Compatibility

This watchface is built using SDK 4.0 and supports multiple Pebble platforms, specifically optimized for color displays and health tracking:
* Pebble Time (Basalt)
* Pebble Time Steel (Basalt)
* Pebble 2 (Diorite)
* Pebble Time 2 (Emery)

## 🛠 Tech Stack

* **C (Pebble SDK):** Core watchface logic, UI rendering, and memory management.
* **JavaScript (PebbleKit JS):** Secure HTTP polling to the Last.fm API and AppMessage queue management.
* **Clay:** For the clean, native-feeling configuration page.

---
*proudly made by [atomlabor.de](https://atomlabor.de)*
