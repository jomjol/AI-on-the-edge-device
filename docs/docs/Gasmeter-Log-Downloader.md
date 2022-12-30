## **Gasmeter Log-Downloader**

This small tool downloads the logfiles from your ESP32 and stores the last value of the day in an *.csv file.

To use this tool you need to **activate the debug logfile** in your configuration (Configuration / Debug / Logfile). I go with 30 days of retention in days.

It downloads only the past logfiles (yesterday and older).

You can define the max. number of Logfiles to download (beginning from newest [yesterday]).

I wrote this tool to get a chart of the daily gas consumption to optimize my gas powered heating.

**Variables to define by yourself:**

- **URL to Logfile-Path on Device:** "http://ESP32-IP-Address/fileserver/log/message/"
- **Download Logfiles to:** enter a valid directory, e.g. "D:\Gaszaehler\Auswertung\Log-Downloads\"
- **Output CSV-File:** enter a valid directory, e.g. "D:\Gaszaehler\Auswertung\DailyValues.csv"
- **Download Logfiles from past # days:** enter the max. number of logfiles you want to download (<= your logfile retention value in your device configuration)

Feel free to optimize and modify it.