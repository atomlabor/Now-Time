module.exports = [
  { "type": "heading", "defaultValue": "NOW: Time v2.0.1" },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "📖 Tutorial & Help" },
      { "type": "text", "defaultValue": "⌚️ Shake/Tap: Flick your wrist firmly to hide the date and open the music player. After 12 seconds, the watchface will automatically switch back." },
      { "type": "text", "defaultValue": "🎵 Last.fm: Enter your username below. Your profile must be public! The watch automatically checks every 20s if a new song is playing." }
    ]
  },
  { 
    "type": "section", 
    "items": [
      { "type": "heading", "defaultValue": "🎨 Design & Feedback" },
      { 
        "type": "color", 
        "messageKey": "ID_HEADER_COLOR",
        "defaultValue": "0055AA", 
        "label": "Header Color", 
        "sunlight": true 
      },
      {
        "type": "toggle",
        "messageKey": "ID_VIBRATE",
        "label": "Vibrate on new song",
        "defaultValue": true
      }
    ]
  },
  { 
    "type": "section", 
    "items": [
      { "type": "heading", "defaultValue": "🎧 Music Setup" },
      { 
        "type": "input", 
        "messageKey": "ID_LASTFM_USER",
        "label": "Last.fm Username" 
      },
      { 
        "type": "input", 
        "messageKey": "ID_FAV_SONG",
        "defaultValue": "Wonderwall", 
        "label": "Fallback Song (When paused)" 
      }
    ]
  },
  { "type": "text", "defaultValue": "proudly made by atomlabor.de" },
  { "type": "submit", "defaultValue": "Save" }
];