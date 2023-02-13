<div align="center">
  <a href="https://deniz.demirdelen.net/">
    <img src="https://content.demirdelen.net/images/deniz-full-img.png" alt="Logo" width="247" height="100">
  </a>
  <br>
  <br>
</div>

## Arduino ESP8266 Deneyi

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
 
<br>

#
  
<div align="center">
  <br>
  © 2023 Deniz DEMIRDELEN
  <br>
  Bu deney, api.demirdelen.net tarafından desteklenmektedir.
</div>
