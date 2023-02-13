//  (               )   (         )     (               *      (      (      (             (               )
//  )\ )         ( /(   )\ )   ( /(     )\ )          (  `     )\ )   )\ )   )\ )          )\ )         ( /(
// (()/(   (     )\()) (()/(   )\())   (()/(    (     )\))(   (()/(  (()/(  (()/(    (    (()/(   (     )\())
//  /(_))  )\   ((_)\   /(_)) ((_)\     /(_))   )\   ((_)()\   /(_))  /(_))  /(_))   )\    /(_))  )\   ((_)/
//  _(_)  ((_)   _((_) (_))    _((_)   (_))_   ((_)  (_()((_) (_))   (_))   (_))_   ((_)  (_))   ((_)   _((_)
// |   \  | __| | \| | |_ _|  |_  /     |   \  | __| |  \/  | |_ _|  | _ \   |   \  | __| | |    | __| | \| |
// | |) | | _|  | .` |  | |    / /      | |) | | _|  | |\/| |  | |   |   /   | |) | | _|  | |__  | _|  | .` |
// |___/  |___| |_|\_| |___|  /___|     |___/  |___| |_|  |_| |___|  |_|_\   |___/  |___| |____| |___| |_|\_|
//
// Copyright (c) 2023 Deniz DEMIRDELEN
//
// This work is licensed under the terms of the MIT license.  
// For a copy, see https://mit-license.org.

const express = require('express');
const fs = require('fs').promises;
const app = express();

//w3hLUCppceviC3UyA4hw0M2qFqQJi1v4 -- Arduino 
//2hhpKBRkLrr2olCMM8HTUpllqLZWL2BG -- Web
const keys = ["w3hLUCppceviC3UyA4hw0M2qFqQJi1v4", "2hhpKBRkLrr2olCMM8HTUpllqLZWL2BG"];
const statusFile = "status.json";
const retryCount = 5;

app.listen(8080, () => {
  console.log(`Listening on port 8080`);
});

app.get('/esp', async (req, res) => {
  const slowMode = req.query.slow;
  const apiKey = req.query.key;
  let status;

  res.setHeader("Access-Control-Allow-Origin", "*");

  //API anahtarı geçerli mi?
  if (!keys.includes(apiKey)) {
    res.sendStatus(401);
    return;
  }

  //istek geçerli mi? (web için sadece tek bir parametre, arduino için slowMode varsa)
  if (req.originalUrl.includes('&') && apiKey == keys[0] && !slowMode) {
    res.sendStatus(400);
    return;
  }

  try {
    status = await readStatus();
  } catch(error) {
    console.log(error);
    res.sendStatus(500);
    return;
  }

  //isteği gönderen arduino ise
  if (apiKey == keys[0] && slowMode) {
    const lastSeen = new Date().getTime();
    status.slowMode = slowMode == "1";
    status.lastSeen = lastSeen;
    status.sending = false;

    try {
      await writeStatus(status);
    } catch (error) {
      console.log(error);
    }

    res.send(`led:${status.led ? 1 : 0}`);
    return;
  }

  //isteği gönderen web ise 
  if (apiKey == keys[1]) {
    var diff = (new Date().getTime() - status.lastSeen) / 1000;

    res.json({
      led: status.led,
      ledSending: status.sending,
      slowMode: status.slowMode,
      lastSeen: diff,
      isOnline: status.slowMode ? diff <= 60 : diff <= 30
    });
    
    return;
  }

  res.sendStatus(501);
});

app.post('/esp', async (req, res) => {
  const apiKey = req.query.key;
  const ledReq = req.query.led;
  let status;

  res.setHeader("Access-Control-Allow-Origin", "*");
  
  if (!keys.includes(apiKey)) {
    res.sendStatus(401);
    return;
  }

  //istek parametresi geçerli mi
  if (!ledReq || ledReq != 1 && ledReq != 0) {
    res.sendStatus(400);
    return;
  }

  try {
    status = await readStatus();
  } catch(error) {
    console.log(error);
    res.sendStatus(500);
    return;
  }

  //zaten başka bir istek beklemedeyse
  if (status.sending) {
    res.sendStatus(409);
    return;
  }

  var diff = (new Date().getTime() - status.lastSeen) / 1000;

  //Arduino (ve ESP modülü) online değilse
  if (status.slowMode ? diff > 60 : diff > 30) {
    res.sendStatus(403);
    return;
  }

  //zaten öyle ayarlanmışsa tekrar dosyaya yazma.
  if (ledReq == 1 && status.led || ledReq == 0 && !status.led) {
    res.sendStatus(200);
    return;
  }

  status.led = ledReq == 1;
  status.sending = true;

  try {
    await writeStatus(status);
  } catch (error) {
    console.log(error);
  }

  res.sendStatus(200);
  return;
});

//HTTP istekleri paralı, bari varolan isteği kurtaralım :)
//Anlık durumu JSON dosyasından oku, hemde olmazsa tekrar denemeli
//4 saat bunun için uğraştım, sadece fs.promise geçmek ve await kullanmak gerekiyormuş...
function readStatus() {
  return new Promise(async (resolve, reject) => {
    var retries = 0;
    
    while(retries < retryCount) {
      try { 
        resolve(JSON.parse(await fs.readFile(statusFile)));
        break;
      } catch (error) {
        console.log(`JSON okunurken hata oluştu. : ${error}`)
        retries++;
      }

      await wait(250);
    }

    if (retries == retryCount) {
      console.log(`JSON 5 tekrardan sonra bile okunamadı.`);
      reject(`OutOfRetries`);
    }
  })
}

//Anlık durumu JSON dosyasına yaz
function writeStatus(status) {
  return new Promise(async (resolve, reject) => {
    var retries = 0;

    while(retries < retryCount) {
      try { 
        resolve(await fs.writeFile(statusFile, JSON.stringify(status)))
        break;
      } catch (error) {
        console.log(`JSON yazılırken hata oluştu. : ${error}`)
        retries++;
      }

      await wait(250);
    }

    if (retries == retryCount) {
      console.log(`JSON 5 tekrardan sonra bile yazılamadı.`);
      reject(`OutOfRetries`);
    }
  })
}

//async bekleme
function wait(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}