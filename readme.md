## What?
This project is for a budget friendly, unlimited control range, FPV car.

Bill of materials:
| Item | Price ($CAD) | Link |
|---|---|---|
| Waveshare ESP32-S3 MINI | 15 | [Amazon](https://www.amazon.ca/Waveshare-Development-ESP32-C6FH4-Processors-Frequency/dp/B0CZKM6HVH/ref=sr_1_10?crid=3986S01I0ODQR&dib=eyJ2IjoiMSJ9.rwc7Jthi8UOgGLGTdx5ArF-SJgfhuzdkrt-raBK-RKyJg8berLR6tnK_rFTWvUIFNwbRWtgf2gaF3tMxfgdHUpYFL0sCqdHtRVLsUUOKDB0gcgGBZLk9N899jgpEWgiGJj-3IYZGFbN-iuZLonV4wZb7SlQWUwAv4hZYarlRcudPxEUg8DiY8sdBsOLZ3NnkQUWS5FtLxozqkALuuj2S53thyCK557-T9_xw09IOSuqb_nHDYXR1ky2Q7N-55IzHeK9MFhOSLO42QK3w2C9YPwOPXLCSRGYJr-sEzpWp23xUVMAY-82oGh7OMFausGJX2Q_kBvMaxq9xScWokYUUMCSvdqwlKnhnUchc_vMufT8.TEJ3oA-U_95Hm7j-1m7SM4fexN5oCe8jLTWwvERAge0&dib_tag=se&keywords=esp32s3+mini&qid=1734548848&s=electronics&sprefix=esp32s3+mini%2Celectronics%2C88&sr=1-10) |
| RC Kei truck (WPL D14) | 80 | [Amazon](https://www.amazon.ca/dp/B0CBPXJCB3?ref=ppx_yo2ov_dt_b_fed_asin_title) |
| RC car ESC for WPLC14 | 20 | [Amazon](https://www.amazon.ca/gp/product/B08FBVBKGN/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1) |
| Your old phone | Free.99 | Check your drawers |
| Solder iron | Free.99 | Check tool box |
| 1 transistor PNP | Free.99 | Check under mattress |
| Hook-up wires | Free.99 | Check night stand #1 |
| Perfboard | Free.99 | Check drawers |
| Xbox controller or Sim racing wheel (optional) | 50 | Facebook marketplace

## How?
### Sever
This is the websocket server that will run on your computer and control the RC car. It is built with Electron JS.
- Install node js on your computer.
- Go into /Server and run `npm install`.
- Start the server with `npm start`.
- Go to your modem control panel and forward port 8000.

### RC car
- Change the values in **config_example.h** to match your setup, and rename the file to **config.h**.
- With the Arduino IDE, compile and upload **TruckScript/TruckScript.ino**
- From your computer, create a room in https://vdo.ninja/ and join the room from your old phone, creating a live video feed.
- Securely fasten your phone to the RC car
- 🚚💨

## Who?
This project was made by [Alex Chang](https://www.linkedin.com/in/alexyuchang/). 