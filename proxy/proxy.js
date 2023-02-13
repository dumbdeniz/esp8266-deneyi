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
const https = require('https');
const app = express();

const url = "https://api.demirdelen.net/esp?slow={slowMode}&key=w3hLUCppceviC3UyA4hw0M2qFqQJi1v4"
const options = {
    headers: {
        "User-Agent": "Arduino/ESP8266",
        "Connection": "close"
    }
};

app.listen(80, () => {
    console.log("Proxy listening on port 80");
})

app.get('/esp-proxy', async (req, res) => {
    const slowMode = req.query.slow;
    let status;

    if (!slowMode) {
        res.sendStatus(400);
        return;
    }

    try {
        status = await sendRequest(slowMode);
    } catch (error) {
        res.sendStatus(500);
        console.log(error);
        return;
    }

    res.setHeader("Connection", "close");
    res.send(status);
})

function sendRequest(slowMode) {
    return new Promise(async (resolve, reject) => {
        const requestUrl = url.replace("{slowMode}", slowMode == "1" ? "1" : "0");

        https.get(requestUrl, options, (resp) => {
            let data = '';

            resp.on('data', (chunk) => data += chunk);

            resp.on('end', () => resolve(data));

        }).on('error', (err) => {
            reject(err);
        });
    });
}