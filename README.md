<div align="center">
  <a href="https://deniz.demirdelen.net/">
    <img src="https://content.demirdelen.net/images/deniz-full-img.png" alt="Logo" width="247" height="100">
  </a>
  <br>
  <br>
</div>

## Genel Bakış

* Arduino
  - ESP8266 modülünü gerekli ayarlarda başlatma, tanımlanan Wi-Fi ağına bağlama
  - Seri iletişim ile modüle AT komutları gönderme, sonuçlarını doğrulama
  - HTTP istekleri gönderme ve yanıta göre bağlı LED'i yakıp söndürme
  - Kod hatalarına karşı Arduino'yu oto. yeniden başlatma
  - Hata ayıklamayı kolaylaştıran, okunabilir seri çıktısı.
  - Akıllı İstek Azaltıcı™ ile sunucuya gereksiz istekler göndermekten kaçınma
  <br>
  
* Sunucu
  - Node.js ve Express.js kullanılarak yazılmış REST API
  - API anahtarları ile doğrulama
  - API anahtarına göre isteği gönderen cihazı bulma
  - İsteklerin ne zaman gönderildiğini takip ederek Arduino'nun çevrimiçi olup olmadığını kontrol etme.
  - Arduino çevrımdışı ise LED değiştirme isteklerini reddetme
  - Arduino var olan LED değiştirme isteğini okumadığı sürece yeni değiştirme isteklerini reddetme
  - Arduino'nun var olan LED değiştirme isteğinini okuyup okumadığını belirten yanıt değeri
  - Arduino'nun yavaş modda olup olmadığını belirten yanıt değeri
  - Anlık durumu JSON dosyasına kaydetme, bu sırada herhangi bir hata oluşması durumunda yeniden deneme
  - Anlık durumun gelen istekle aynı olması gibi durumlarda herhangi bir işlem yapmamak gibi performans optimizasyonları
  <br>
  
* Web
  - Fetch kullanarak javascript üzerinden HTTP istekleri gönderme
  - Polling ile canlı veri sağlama
  - Anlık durumu kontrol etme (açık, uykuda, çevrimdışı vb.)
  - LED durumunu okuma ve değiştirme
  - Responsive arayüz
  - Aura Parıltısı™ ile LED'in açık olduğunu belirten ışık efekti
<br>

## API Kullanımı

### Arduino
```
GET https://api.demirdelen.net/esp?slow=0&key=w3hLUCppceviC3UyA4hw0M2qFqQJi1v4
```
&emsp; ```slow=[0-1]``` <br>

&emsp; &emsp; İstek azaltıcı yavaş modun çalışma durumunu belirtir. Böylece Arduino çevrımdışı sayılmadan 60 saniye boyunca uykuya girebilir.

#### Yanıt

```
led:0
```

&emsp; ```led:[0-1]``` <br>

&emsp; &emsp; Arduino'nun LED'i yakıp yakmaması gerektiğini belirtir.

#

### Web
```
GET https://api.demirdelen.net/esp?key=2hhpKBRkLrr2olCMM8HTUpllqLZWL2BG
```

#### Yanıt

```
{
    "led": false,
    "ledSending": false,
    "slowMode": false,
    "lastSeen": 1.23,
    "isOnline": true
}
```

&emsp; ```led:``` <br>

&emsp; &emsp; Sunucudaki LED durumunu belirtir. Bu değer LED değiştirme isteğinden sonra hemen değişir ancak Arduino'nun bu değeri okuyup işlemesi birkaç saniye (veya daha fazla) süreceğinden şu anlık LED durumunu değil, olması gereken durumu belirtir.

&emsp; ```ledSending:``` <br>

&emsp; &emsp; Arduino, sunucudaki LED durumunu okuyup işleyene kadar değeri ```true``` olur. İsteğin Arduino'ya ulaşıp ulaşmadığını yorumlamak için kullanılabilir ancak işlemin başarılı olup olmadığını belirtemez.

<br>

#

<div align="center">
  <br>
  © 2023 Deniz DEMIRDELEN
  <br>
  Bu deney, api.demirdelen.net tarafından desteklenmektedir.
</div>
