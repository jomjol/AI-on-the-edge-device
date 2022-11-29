<h2><strong>Gasmeter Value History Downloader</strong></h2>
<p>This small tool downloads the datafiles (*.txt, before V13.0.1) or valuefiles (*.csv, since V13.0.1) from your ESP32 and stores the last value of the day in a *.csv file.</p>
<p>To use this tool you need to <strong>activate the DataLogging</strong> in your configuration (Configuration / Data Logging / DataLogActive). I go with 30 days of retention in days.</p>
<p>It downloads only the past datafiles (yesterday and older, not the actual day).</p>
<p>You can define the max. number of datafiles to download (beginning from newest [yesterday]).</p>
<p>I wrote this tool to get a chart of the daily gas consumption to optimize my gas powered heating.</p>
<p><strong>Variables to define by yourself:</strong></p>
<ul>
<li><strong>URL to Logfile-Path on Device:</strong> "http://ESP32-IP-Address/fileserver/log/"</li>
<li><strong>Download datafiles to:</strong> enter a valid directory, e.g. "D:\Gaszaehler\Auswertung\Log-Downloads\"</li>
<li><strong>Output CSV-File:</strong> enter a valid directory, e.g. "D:\Gaszaehler\Auswertung\DailyValues.csv"</li>
<li><strong>Download past # days:</strong> enter the max. number of days you want to download (&lt;= your datafiles retention value in your device configuration)</li>
</ul>
<p>Feel free to optimize and modify it.</p>