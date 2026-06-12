const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const DiscordRPC = require('discord-rpc');

let mainWindow;
const clientId = '1515011986021421218'; // Your Discord App ID

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1300,
    height: 850,
    backgroundColor: '#06040a',
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false
    }
  });

  mainWindow.setMenu(null);
  mainWindow.loadFile('index.html');
}

// Discord Rich Presence Setup
DiscordRPC.register(clientId);
const rpc = new DiscordRPC.Client({ transport: 'ipc' });

function setActivity(trackName = "Idle / Deck Empty") {
  if (!rpc || !mainWindow) return;

  rpc.setActivity({
    details: 'Larping',
    state: trackName,
    largeImageKey: 'larpmedia',
    largeImageText: 'LarpMedia v1.0',
    instance: true,
    // Type 2 forces the "Listening to..." layout inside Discord status updates
    type: 2 
  }).catch(err => console.error("Discord Presence Error: ", err));
}

rpc.on('ready', () => {
  setActivity();
});

// Communication channel listening to track changes from our HTML5 player deck
ipcMain.on('update-track', (event, trackName) => {
  setActivity(trackName);
});

app.whenReady().then(() => {
  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow();
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    rpc.destroy();
    app.quit();
  }
});
