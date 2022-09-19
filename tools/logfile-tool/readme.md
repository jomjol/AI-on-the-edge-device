<h2><strong>Gasmeter Log-Downloader</strong></h2>
<p>This small tool downloads the logfiles from your ESP32 and stores the last value of the day in an *.csv file.</p>
<p>To use this tool you need to <strong>activate the debug logfile</strong> in your configuration (Configuration / Debug / Logfile). I go with 30 days of retention in days.</p>
<p>It downloads only the past logfiles (yesterday and older).</p>
<p>You can define the max. number of Logfiles to download (beginning from newest [yesterday]).</p>
<p>I wrote this tool to get a chart of the daily gas consumption to optimize my gas powered heating.</p>
<p><strong>Variables to define by yourself:</strong></p>
<ul>
<li><strong>URL to Logfile-Path on Device:</strong> "http://ESP32-IP-Address/fileserver/log/message/"</li>
<li><strong>Download Logfiles to:</strong> enter a valid directory, e.g. "D:\Gaszaehler\Auswertung\Log-Downloads\"</li>
<li><strong>Output CSV-File:</strong> enter a valid directory, e.g. "D:\Gaszaehler\Auswertung\DailyValues.csv"</li>
<li><strong>Download Logfiles from past # days:</strong> enter the max. number of logfiles you want to download (&lt;= your logfile retention value in your device configuration)</li>
</ul>
<p>Feel free to optimize and modify it.</p>