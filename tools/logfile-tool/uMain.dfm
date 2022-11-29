object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'Gasmeter Value-History'
  ClientHeight = 523
  ClientWidth = 453
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 13
  object lblImpressum: TLabel
    Left = 316
    Top = 468
    Width = 68
    Height = 13
    Caption = 'reserve, 2022'
  end
  object lbledtURL: TLabeledEdit
    Left = 28
    Top = 36
    Width = 273
    Height = 21
    EditLabel.Width = 146
    EditLabel.Height = 13
    EditLabel.Caption = 'URL to Logfile-Path on Device:'
    TabOrder = 0
    Text = 'http://192.168.10.65/fileserver/log/'
  end
  object btnDownloadLogfiles: TButton
    Left = 28
    Top = 174
    Width = 273
    Height = 25
    Caption = 'Download datafiles and generate CSV'
    TabOrder = 1
    OnClick = btnDownloadLogfilesClick
  end
  object lbledtMaxLogfilesOnServer: TLabeledEdit
    Left = 323
    Top = 36
    Width = 114
    Height = 21
    EditLabel.Width = 112
    EditLabel.Height = 13
    EditLabel.Caption = 'Download past # days:'
    TabOrder = 2
    Text = '30'
  end
  object lbledtTargetDirectory: TLabeledEdit
    Left = 28
    Top = 84
    Width = 273
    Height = 21
    EditLabel.Width = 108
    EditLabel.Height = 13
    EditLabel.Caption = 'Download datafiles to:'
    TabOrder = 3
    Text = 'C:\Temp\Gas\Log\'
  end
  object lbledtCsvFile: TLabeledEdit
    Left = 28
    Top = 131
    Width = 273
    Height = 21
    EditLabel.Width = 80
    EditLabel.Height = 13
    EditLabel.Caption = 'Output CSV-File:'
    TabOrder = 4
    Text = 'C:\Temp\Gas\Values.csv'
  end
  object redtLog: TRichEdit
    Left = 28
    Top = 220
    Width = 273
    Height = 277
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Lines.Strings = (
      'redtLog')
    ParentFont = False
    TabOrder = 5
    Zoom = 100
  end
  object idhtp1: TIdHTTP
    AllowCookies = True
    ProxyParams.BasicAuthentication = False
    ProxyParams.ProxyPort = 0
    Request.ContentLength = -1
    Request.ContentRangeEnd = -1
    Request.ContentRangeStart = -1
    Request.ContentRangeInstanceLength = -1
    Request.Accept = 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8'
    Request.BasicAuthentication = False
    Request.UserAgent = 'Mozilla/3.0 (compatible; Indy Library)'
    Request.Ranges.Units = 'bytes'
    Request.Ranges = <>
    HTTPOptions = [hoForceEncodeParams]
    Left = 572
    Top = 464
  end
end
