var Clay = require('@rebble/clay');
var clayConfig = require('./config.js');
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

var API_KEY = "59ce949841502d8fba7c9253bc11bfa9";

var msgQueue = [];
var msgBusy = false;

function sendMessage(dict) {
  msgQueue.push(dict);
  drainQueue();
}

function drainQueue() {
  if (msgBusy || msgQueue.length === 0) return;
  msgBusy = true;
  var msg = msgQueue.shift();
  Pebble.sendAppMessage(
    msg,
    function() { msgBusy = false; drainQueue(); },
    function(err) {
      console.log('[NACK] ' + JSON.stringify(err));
      msgBusy = false;
      drainQueue();
    }
  );
}

var pollTimer = null;
var currentUser = null;
var lastSentTrack = null; 

function schedulePoll(user, delayMs) {
  if (pollTimer) clearTimeout(pollTimer);
  currentUser = user;
  pollTimer = setTimeout(function() {
    pollTimer = null;
    fetchCurrentTrack(user);
  }, delayMs || 0);
}

Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) return;

  var settings;
  try {
    settings = clay.getSettings(e.response, false);
  } catch (err) {
    console.log('[Clay] Abbruch durch User oder Parse-Fehler.');
    return; 
  }

  var user    = settings['ID_LASTFM_USER'] ? String(settings['ID_LASTFM_USER'].value).trim() : '';
  var favSong = settings['ID_FAV_SONG']    ? String(settings['ID_FAV_SONG'].value)            : 'Wonderwall';
  
  var colorVal = settings['ID_HEADER_COLOR'] ? settings['ID_HEADER_COLOR'].value : 0x0055AA;
  var colorInt;
  if (typeof colorVal === 'string' && colorVal.match(/^[0-9a-fA-F]{6}$/)) {
    colorInt = parseInt(colorVal, 16);
  } else {
    colorInt = Number(colorVal); 
  }
  if (isNaN(colorInt)) colorInt = 0x0055AA;

  // Toggle für Vibration als 1 oder 0 senden
  var vibrate = settings['ID_VIBRATE'] ? settings['ID_VIBRATE'].value : true;

  if (user) localStorage.setItem('lastfm_user', user);

  sendMessage({
    'ID_FAV_SONG':     favSong,
    'ID_HEADER_COLOR': colorInt,
    'ID_LASTFM_USER':  user,
    'ID_VIBRATE':      vibrate ? 1 : 0
  });

  lastSentTrack = null; 
  if (user) schedulePoll(user, 1000); 
});

// --- APP START ---
Pebble.addEventListener('ready', function() {
  var user = localStorage.getItem('lastfm_user');
  if (user) schedulePoll(user, 2000);
});

// --- LAST.FM FETCH ---
function fetchCurrentTrack(user) {
  if (!user) return;

  var url = 'https://ws.audioscrobbler.com/2.0/?method=user.getrecenttracks&user=' +
            encodeURIComponent(user) +
            '&api_key=' + API_KEY + '&format=json&limit=1';

  var req = new XMLHttpRequest();
  req.open('GET', url, true);

  req.onload = function() {
    if (req.status === 200) {
      try {
        var json    = JSON.parse(req.responseText);
        if (!json || !json.recenttracks || !json.recenttracks.track) {
           schedulePoll(user, 30000);
           return;
        }

        var tracks  = json.recenttracks.track;
        var track   = Array.isArray(tracks) ? tracks[0] : tracks;
        var playing = track && track['@attr'] && track['@attr'].nowplaying === 'true';
        var info    = playing ? (track.name + ' - ' + track.artist['#text']) : '';

        if (info !== lastSentTrack) {
          sendMessage({ 'ID_TRACK_NAME': info }); 
          lastSentTrack = info;
        }
      } catch (ex) {
        console.log('[Last.fm] Parse-Fehler: ' + ex);
      }
    }
    schedulePoll(user, 20000);
  };

  req.onerror = function() {
    schedulePoll(user, 30000);
  };

  req.send(null);
}