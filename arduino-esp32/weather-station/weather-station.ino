#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define USE_SERIAL Serial

// Pinout ESP32Wroom Doit V1 https://github.com/playelek/pinout-doit-32devkitv1

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiMulti wifiMulti;

const char* wifiSsid     = "";
const char* wifiPassword = "";

const char* weatherApiKey = "";
//const char* weatherUrl = "https://api.openweathermap.org/data/2.5/onecall?lat={lat}&lon={lon}&units=metric&exclude=hourly,daily&appid={appid}";
const char* weatherUrl = "https://api.openweathermap.org/data/2.5/weather?units=metric&appid=";

// openssl s_client -connect api.openweathermap.org:443 -showcerts -verify 5
const char* weatherCaCert = \ 
"-----BEGIN CERTIFICATE-----\n" \ 
"MIIGvjCCBaagAwIBAgIRAKL7IEo7D+v9u0G4ItYCJYwwDQYJKoZIhvcNAQELBQAw\n" \ 
"gY8xCzAJBgNVBAYTAkdCMRswGQYDVQQIExJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\n" \ 
"BgNVBAcTB1NhbGZvcmQxGDAWBgNVBAoTD1NlY3RpZ28gTGltaXRlZDE3MDUGA1UE\n" \ 
"AxMuU2VjdGlnbyBSU0EgRG9tYWluIFZhbGlkYXRpb24gU2VjdXJlIFNlcnZlciBD\n" \ 
"QTAeFw0yMDAzMTcwMDAwMDBaFw0yMjA2MTkwMDAwMDBaMB8xHTAbBgNVBAMMFCou\n" \ 
"b3BlbndlYXRoZXJtYXAub3JnMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n" \ 
"AQEA2DMTq6QbiQ6N/PK6u6dv8J1w5/w/GLm1d7J3daL80/15qRlsxUEpM78/OWmE\n" \ 
"s60kKSfyOVyxOHrVoXMfEhIxATdYQtRtN2JQEFYDkRauvVgr5eXQO2EJZXBZUb2C\n" \ 
"0dLFMD2WtrQGl7059kCOBlA/vX2+uTIQwFx/qZyVKkhzgdthtoDQ5jDzx7scDM0U\n" \ 
"9c/be/aWNPzoJV1HK37luC0nHUyT0zDpXMt82DgoCRix9z9RzDNkyjsPW2qP/pOE\n" \ 
"RpXk0z49jOFqtUxTtR9HfbKoeQ/RobxD2fG5P1cfunZ2lU3lyl5PeKbmMlSdSlci\n" \ 
"4OuileGdauTqgU254X7bB/9iTQIDAQABo4IDgjCCA34wHwYDVR0jBBgwFoAUjYxe\n" \ 
"xFStiuF36Zv5mwXhuAGNYeEwHQYDVR0OBBYEFP2HTXuP9/WVxbHQk4RHPpXCLktU\n" \ 
"MA4GA1UdDwEB/wQEAwIFoDAMBgNVHRMBAf8EAjAAMB0GA1UdJQQWMBQGCCsGAQUF\n" \ 
"BwMBBggrBgEFBQcDAjBJBgNVHSAEQjBAMDQGCysGAQQBsjEBAgIHMCUwIwYIKwYB\n" \ 
"BQUHAgEWF2h0dHBzOi8vc2VjdGlnby5jb20vQ1BTMAgGBmeBDAECATCBhAYIKwYB\n" \ 
"BQUHAQEEeDB2ME8GCCsGAQUFBzAChkNodHRwOi8vY3J0LnNlY3RpZ28uY29tL1Nl\n" \ 
"Y3RpZ29SU0FEb21haW5WYWxpZGF0aW9uU2VjdXJlU2VydmVyQ0EuY3J0MCMGCCsG\n" \ 
"AQUFBzABhhdodHRwOi8vb2NzcC5zZWN0aWdvLmNvbTAzBgNVHREELDAqghQqLm9w\n" \ 
"ZW53ZWF0aGVybWFwLm9yZ4ISb3BlbndlYXRoZXJtYXAub3JnMIIB9gYKKwYBBAHW\n" \ 
"eQIEAgSCAeYEggHiAeAAdwBGpVXrdfqRIDC1oolp9PN9ESxBdL79SbiFq/L8cP5t\n" \ 
"RwAAAXDobGj8AAAEAwBIMEYCIQDuoxRU3qxvOhsXh/vQPwAzBQfmu0b76RYKY27r\n" \ 
"3IjeuwIhAKhiaG0C9WMqsBNviTNJHl8iUZppSoDbreFWKU3ju715AHYA36Veq2iC\n" \ 
"Tx9sre64X04+WurNohKkal6OOxLAIERcKnMAAAFw6GxoyAAABAMARzBFAiEAiPLZ\n" \ 
"oR9BVGbeBKcZWWCWe5khT1jrbwqFFs1qqciHhmUCICNPG3dRIueExiu3HF6tUiNb\n" \ 
"rlGF/mf9Efr3JkAkqGsZAHUAQcjKsd8iRkoQxqE6CUKHXk4xixsD6+tLx2jwkGKW\n" \ 
"BvYAAAFw6Gxo7AAABAMARjBEAiAzzodBqseRU0wn7ukh37SvTOjmv8vpayKuZ4AE\n" \ 
"ut06BAIgArnrQObBVZU87a6ubmSWGHPiEi8cyPYdqZkMVycT3TgAdgBvU3asMfAx\n" \ 
"GdiZAKRRFf93FRwR2QLBACkGjbIImjfZEwAAAXDobGnaAAAEAwBHMEUCIGo9M7aa\n" \ 
"TjzbYPbR16+gwPnAGNiZI0ujRTDXRUJsW+D8AiEAgexT/9i23R7/XZfh5sL1Q9E/\n" \ 
"pE40zy1wXC1O3BHvz2MwDQYJKoZIhvcNAQELBQADggEBANJ4pa0tYp5QOtGy1RxM\n" \ 
"hcX2WydaU89WwySUB41pxbXBvaRLQyFBzC/COjPyN6zR52irYeBr0uFLLmwkaZfg\n" \ 
"eavkaExosslVP9g1js4j7wAKR5CdlEJfgw4eTxu8LAx5WUhm66HaMQol2neSyky2\n" \ 
"XPZt4KvZC9Fk/0x28JpXbMpckpH1/VpWPz3ulQw1/9TgV0+saRpFaKVXoZT5IObo\n" \ 
"j6cAp85OGBmRNJFypFFZRvy85aPJCP8IIyNoC9MoZIQ2VEuXQMTrIDU14Y46BTDq\n" \ 
"HaolM6WQZl42iGBzqJcOF2PGzcZ5YUahZW1GMxwB3NCyugR93FMCwtM4Wip6Ja5Q\n" \ 
"5fs=\n" \ 
"-----END CERTIFICATE-----\n" \ 
"-----BEGIN CERTIFICATE-----\n" \ 
"MIIGEzCCA/ugAwIBAgIQfVtRJrR2uhHbdBYLvFMNpzANBgkqhkiG9w0BAQwFADCB\n" \ 
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \ 
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \ 
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTgx\n" \ 
"MTAyMDAwMDAwWhcNMzAxMjMxMjM1OTU5WjCBjzELMAkGA1UEBhMCR0IxGzAZBgNV\n" \ 
"BAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEYMBYGA1UE\n" \ 
"ChMPU2VjdGlnbyBMaW1pdGVkMTcwNQYDVQQDEy5TZWN0aWdvIFJTQSBEb21haW4g\n" \ 
"VmFsaWRhdGlvbiBTZWN1cmUgU2VydmVyIENBMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \ 
"AQ8AMIIBCgKCAQEA1nMz1tc8INAA0hdFuNY+B6I/x0HuMjDJsGz99J/LEpgPLT+N\n" \ 
"TQEMgg8Xf2Iu6bhIefsWg06t1zIlk7cHv7lQP6lMw0Aq6Tn/2YHKHxYyQdqAJrkj\n" \ 
"eocgHuP/IJo8lURvh3UGkEC0MpMWCRAIIz7S3YcPb11RFGoKacVPAXJpz9OTTG0E\n" \ 
"oKMbgn6xmrntxZ7FN3ifmgg0+1YuWMQJDgZkW7w33PGfKGioVrCSo1yfu4iYCBsk\n" \ 
"Haswha6vsC6eep3BwEIc4gLw6uBK0u+QDrTBQBbwb4VCSmT3pDCg/r8uoydajotY\n" \ 
"uK3DGReEY+1vVv2Dy2A0xHS+5p3b4eTlygxfFQIDAQABo4IBbjCCAWowHwYDVR0j\n" \ 
"BBgwFoAUU3m/WqorSs9UgOHYm8Cd8rIDZsswHQYDVR0OBBYEFI2MXsRUrYrhd+mb\n" \ 
"+ZsF4bgBjWHhMA4GA1UdDwEB/wQEAwIBhjASBgNVHRMBAf8ECDAGAQH/AgEAMB0G\n" \ 
"A1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAbBgNVHSAEFDASMAYGBFUdIAAw\n" \ 
"CAYGZ4EMAQIBMFAGA1UdHwRJMEcwRaBDoEGGP2h0dHA6Ly9jcmwudXNlcnRydXN0\n" \ 
"LmNvbS9VU0VSVHJ1c3RSU0FDZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNybDB2Bggr\n" \ 
"BgEFBQcBAQRqMGgwPwYIKwYBBQUHMAKGM2h0dHA6Ly9jcnQudXNlcnRydXN0LmNv\n" \ 
"bS9VU0VSVHJ1c3RSU0FBZGRUcnVzdENBLmNydDAlBggrBgEFBQcwAYYZaHR0cDov\n" \ 
"L29jc3AudXNlcnRydXN0LmNvbTANBgkqhkiG9w0BAQwFAAOCAgEAMr9hvQ5Iw0/H\n" \ 
"ukdN+Jx4GQHcEx2Ab/zDcLRSmjEzmldS+zGea6TvVKqJjUAXaPgREHzSyrHxVYbH\n" \ 
"7rM2kYb2OVG/Rr8PoLq0935JxCo2F57kaDl6r5ROVm+yezu/Coa9zcV3HAO4OLGi\n" \ 
"H19+24rcRki2aArPsrW04jTkZ6k4Zgle0rj8nSg6F0AnwnJOKf0hPHzPE/uWLMUx\n" \ 
"RP0T7dWbqWlod3zu4f+k+TY4CFM5ooQ0nBnzvg6s1SQ36yOoeNDT5++SR2RiOSLv\n" \ 
"xvcRviKFxmZEJCaOEDKNyJOuB56DPi/Z+fVGjmO+wea03KbNIaiGCpXZLoUmGv38\n" \ 
"sbZXQm2V0TP2ORQGgkE49Y9Y3IBbpNV9lXj9p5v//cWoaasm56ekBYdbqbe4oyAL\n" \ 
"l6lFhd2zi+WJN44pDfwGF/Y4QA5C5BIG+3vzxhFoYt/jmPQT2BVPi7Fp2RBgvGQq\n" \ 
"6jG35LWjOhSbJuMLe/0CjraZwTiXWTb2qHSihrZe68Zk6s+go/lunrotEbaGmAhY\n" \ 
"LcmsJWTyXnW0OMGuf1pGg+pRyrbxmRE1a6Vqe8YAsOf4vmSyrcjC8azjUeqkk+B5\n" \ 
"yOGBQMkKW+ESPMFgKuOXwIlCypTPRpgSabuY0MLTDXJLR27lk8QyKGOHQ+SwMj4K\n" \ 
"00u/I5sUKUErmgQfky3xxzlIPK1aEn8=\n" \ 
"-----END CERTIFICATE-----\n" \ 
"-----BEGIN CERTIFICATE-----\n" \ 
"MIIFdzCCBF+gAwIBAgIQE+oocFv07O0MNmMJgGFDNjANBgkqhkiG9w0BAQwFADBv\n" \ 
"MQswCQYDVQQGEwJTRTEUMBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFk\n" \ 
"ZFRydXN0IEV4dGVybmFsIFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBF\n" \ 
"eHRlcm5hbCBDQSBSb290MB4XDTAwMDUzMDEwNDgzOFoXDTIwMDUzMDEwNDgzOFow\n" \ 
"gYgxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpOZXcgSmVyc2V5MRQwEgYDVQQHEwtK\n" \ 
"ZXJzZXkgQ2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMS4wLAYD\n" \ 
"VQQDEyVVU0VSVHJ1c3QgUlNBIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIICIjAN\n" \ 
"BgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAgBJlFzYOw9sIs9CsVw127c0n00yt\n" \ 
"UINh4qogTQktZAnczomfzD2p7PbPwdzx07HWezcoEStH2jnGvDoZtF+mvX2do2NC\n" \ 
"tnbyqTsrkfjib9DsFiCQCT7i6HTJGLSR1GJk23+jBvGIGGqQIjy8/hPwhxR79uQf\n" \ 
"jtTkUcYRZ0YIUcuGFFQ/vDP+fmyc/xadGL1RjjWmp2bIcmfbIWax1Jt4A8BQOujM\n" \ 
"8Ny8nkz+rwWWNR9XWrf/zvk9tyy29lTdyOcSOk2uTIq3XJq0tyA9yn8iNK5+O2hm\n" \ 
"AUTnAU5GU5szYPeUvlM3kHND8zLDU+/bqv50TmnHa4xgk97Exwzf4TKuzJM7UXiV\n" \ 
"Z4vuPVb+DNBpDxsP8yUmazNt925H+nND5X4OpWaxKXwyhGNVicQNwZNUMBkTrNN9\n" \ 
"N6frXTpsNVzbQdcS2qlJC9/YgIoJk2KOtWbPJYjNhLixP6Q5D9kCnusSTJV882sF\n" \ 
"qV4Wg8y4Z+LoE53MW4LTTLPtW//e5XOsIzstAL81VXQJSdhJWBp/kjbmUZIO8yZ9\n" \ 
"HE0XvMnsQybQv0FfQKlERPSZ51eHnlAfV1SoPv10Yy+xUGUJ5lhCLkMaTLTwJUdZ\n" \ 
"+gQek9QmRkpQgbLevni3/GcV4clXhB4PY9bpYrrWX1Uu6lzGKAgEJTm4Diup8kyX\n" \ 
"HAc/DVL17e8vgg8CAwEAAaOB9DCB8TAfBgNVHSMEGDAWgBStvZh6NLQm9/rEJlTv\n" \ 
"A73gJMtUGjAdBgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/\n" \ 
"BAQDAgGGMA8GA1UdEwEB/wQFMAMBAf8wEQYDVR0gBAowCDAGBgRVHSAAMEQGA1Ud\n" \ 
"HwQ9MDswOaA3oDWGM2h0dHA6Ly9jcmwudXNlcnRydXN0LmNvbS9BZGRUcnVzdEV4\n" \ 
"dGVybmFsQ0FSb290LmNybDA1BggrBgEFBQcBAQQpMCcwJQYIKwYBBQUHMAGGGWh0\n" \ 
"dHA6Ly9vY3NwLnVzZXJ0cnVzdC5jb20wDQYJKoZIhvcNAQEMBQADggEBAJNl9jeD\n" \ 
"lQ9ew4IcH9Z35zyKwKoJ8OkLJvHgwmp1ocd5yblSYMgpEg7wrQPWCcR23+WmgZWn\n" \ 
"RtqCV6mVksW2jwMibDN3wXsyF24HzloUQToFJBv2FAY7qCUkDrvMKnXduXBBP3zQ\n" \ 
"YzYhBx9G/2CkkeFnvN4ffhkUyWNnkepnB2u0j4vAbkN9w6GAbLIevFOFfdyQoaS8\n" \ 
"Le9Gclc1Bb+7RrtubTeZtv8jkpHGbkD4jylW6l/VXxRTrPBPYer3IsynVgviuDQf\n" \ 
"Jtl7GQVoP7o81DgGotPmjw7jtHFtQELFhLRAlSv0ZaBIefYdgWOWnU914Ph85I6p\n" \ 
"0fKtirOMxyHNwu8=\n" \ 
"-----END CERTIFICATE-----\n";

// Get it via http://arduinojson.org/assistant/
const size_t weatherBufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(13) + 280;
DynamicJsonDocument weather(weatherBufferSize);

const char* ipapiUrl = "https://ipapi.co/json";

const char* ipapiCaCert = \
"-----BEGIN CERTIFICATE-----\n" \ 
"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n" \ 
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n" \ 
"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n" \ 
"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n" \ 
"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n" \ 
"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n" \ 
"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n" \ 
"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n" \ 
"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n" \ 
"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n" \ 
"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n" \ 
"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n" \ 
"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n" \ 
"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n" \ 
"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n" \ 
"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n" \ 
"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n" \ 
"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n" \ 
"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n" \ 
"-----END CERTIFICATE-----\n" \ 
"-----BEGIN CERTIFICATE-----\n" \ 
"MIIDozCCAougAwIBAgIQD/PmFjmqPRoSZfQfizTltjANBgkqhkiG9w0BAQsFADBa\n" \ 
"MQswCQYDVQQGEwJJRTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJl\n" \ 
"clRydXN0MSIwIAYDVQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTE1\n" \ 
"MTAxNDEyMDAwMFoXDTIwMTAwOTEyMDAwMFowbzELMAkGA1UEBhMCVVMxCzAJBgNV\n" \ 
"BAgTAkNBMRYwFAYDVQQHEw1TYW4gRnJhbmNpc2NvMRkwFwYDVQQKExBDbG91ZEZs\n" \ 
"YXJlLCBJbmMuMSAwHgYDVQQDExdDbG91ZEZsYXJlIEluYyBFQ0MgQ0EtMjBZMBMG\n" \ 
"ByqGSM49AgEGCCqGSM49AwEHA0IABNFW9Jy25DGg9aRSz+Oaeob/8oayXsy1WcwR\n" \ 
"x07dZP1VnGDjoEvZeFT/SFC6ouGhWHWPx2A3RBZNVZns7tQzeiOjggEZMIIBFTAS\n" \ 
"BgNVHRMBAf8ECDAGAQH/AgEAMA4GA1UdDwEB/wQEAwIBhjA0BggrBgEFBQcBAQQo\n" \ 
"MCYwJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTA6BgNVHR8E\n" \ 
"MzAxMC+gLaArhilodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vT21uaXJvb3QyMDI1\n" \ 
"LmNybDA9BgNVHSAENjA0MDIGBFUdIAAwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93\n" \ 
"d3cuZGlnaWNlcnQuY29tL0NQUzAdBgNVHQ4EFgQUPnQtH89FdQR+P8Cihz5MQ4NR\n" \ 
"E8YwHwYDVR0jBBgwFoAU5Z1ZMIJHWMys+ghUNoZ7OrUETfAwDQYJKoZIhvcNAQEL\n" \ 
"BQADggEBADhfp//8hfJzMuTVo4mZlmCvMsEDs2Xfvh4DyqXthbKPr0uMc48qjKkA\n" \ 
"DgEkF/fsUoV2yOUcecrDF4dQtgQzNp4qnhgXljISr0PMVxje28fYiCWD5coGJTH9\n" \ 
"vV1IO1EB3SwUx8FgUemVAdiyM1YOR2aNbM2v+YXZ6xxHR4g06PD6wqtPaU4JWdRX\n" \ 
"xszByOPmGcFYOFLi4oOF3iI03D+m968kvOBvwKtoLVLHawVXLEIbLUiHAwyQq0hI\n" \ 
"qSi+NIr7uu30YJkdFXgRqtltU39pKLy3ayB2f6BVA3F59WensKAKF1eyAKmtz/9n\n" \ 
"jD4m5ackvMJvEOiJxnCl0h+A7Q0/JxM=\n" \ 
"-----END CERTIFICATE-----\n" \ 
"-----BEGIN CERTIFICATE-----\n" \ 
"MIIE3zCCBISgAwIBAgIQC2idH27JvIXV+Bcfh1XWljAKBggqhkjOPQQDAjBvMQsw\n" \ 
"CQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExFjAUBgNVBAcTDVNhbiBGcmFuY2lzY28x\n" \ 
"GTAXBgNVBAoTEENsb3VkRmxhcmUsIEluYy4xIDAeBgNVBAMTF0Nsb3VkRmxhcmUg\n" \ 
"SW5jIEVDQyBDQS0yMB4XDTE5MTExMzAwMDAwMFoXDTIwMTAwOTEyMDAwMFowbTEL\n" \ 
"MAkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRYwFAYDVQQHEw1TYW4gRnJhbmNpc2Nv\n" \ 
"MRkwFwYDVQQKExBDbG91ZGZsYXJlLCBJbmMuMR4wHAYDVQQDExVzbmkuY2xvdWRm\n" \ 
"bGFyZXNzbC5jb20wWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARC8ROUxLvTAf1l\n" \ 
"LX+/3qtOrOmgdlKK2wrQmVnPKd1LxqqbD9EweLw1elj9uWbFVWI1AVfYLh/zXcJ/\n" \ 
"w2SRSkILo4IDAjCCAv4wHwYDVR0jBBgwFoAUPnQtH89FdQR+P8Cihz5MQ4NRE8Yw\n" \ 
"HQYDVR0OBBYEFDf6MuGqA5qv5b5SkS256AKWX4paMDYGA1UdEQQvMC2CCGlwYXBp\n" \ 
"LmNvghVzbmkuY2xvdWRmbGFyZXNzbC5jb22CCiouaXBhcGkuY28wDgYDVR0PAQH/\n" \ 
"BAQDAgeAMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjB5BgNVHR8EcjBw\n" \ 
"MDagNKAyhjBodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vQ2xvdWRGbGFyZUluY0VD\n" \ 
"Q0NBMi5jcmwwNqA0oDKGMGh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9DbG91ZEZs\n" \ 
"YXJlSW5jRUNDQ0EyLmNybDBMBgNVHSAERTBDMDcGCWCGSAGG/WwBATAqMCgGCCsG\n" \ 
"AQUFBwIBFhxodHRwczovL3d3dy5kaWdpY2VydC5jb20vQ1BTMAgGBmeBDAECAjB2\n" \ 
"BggrBgEFBQcBAQRqMGgwJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0\n" \ 
"LmNvbTBABggrBgEFBQcwAoY0aHR0cDovL2NhY2VydHMuZGlnaWNlcnQuY29tL0Ns\n" \ 
"b3VkRmxhcmVJbmNFQ0NDQS0yLmNydDAMBgNVHRMBAf8EAjAAMIIBBAYKKwYBBAHW\n" \ 
"eQIEAgSB9QSB8gDwAHYApLkJkLQYWBSHuxOizGdwCjw1mAT5G9+443fNDsgN3BAA\n" \ 
"AAFuZDSEcQAABAMARzBFAiBxESo7FtV1ptNNfVEW9dkbAMcReS3O3CXvgAUnUNi7\n" \ 
"EQIhALE+/0CBYH6jYUoIORdrNhobki5XsAwdrJHmvjbGupaJAHYAXqdz+d9WwOe1\n" \ 
"Nkh90EngMnqRmgyEoRIShBh1loFxRVgAAAFuZDSEYAAABAMARzBFAiBPQrPMDWMI\n" \ 
"X6Cemibg+7KSy0+w+X9iVTLV497rjyrOggIhAJZqKRDT0tGEpdwDpwHju+++ipw0\n" \ 
"nZTEFIGUDPberxLcMAoGCCqGSM49BAMCA0kAMEYCIQDe1Cp778XSDGUGG2GNYvp/\n" \ 
"utESI4yqq0bJKJcVa50JvwIhAJbzNTPxgHt+HmtoRmZWGXsk+d8WXyLlSrm2AV5S\n" \ 
"O9/e\n" \ 
"-----END CERTIFICATE-----\n";


// Get it via http://arduinojson.org/assistant/
const size_t ipapiBufferSize = JSON_OBJECT_SIZE(25) + 490;
DynamicJsonDocument ipapi(ipapiBufferSize);

const int refreshIntervalSeconds = 60;


void setupWifi()
{
  wifiMulti.addAP(wifiSsid, wifiPassword);
}

// Generated using https://omerk.github.io/lcdchargen/
byte arrowUp[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};
byte arrowDown[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100
};
#define ARROWUP 0
#define ARROWDOWN 1

void setupLcd()
{
  lcd.begin();
  lcd.backlight();
  lcd.createChar(ARROWUP, arrowUp);
  lcd.createChar(ARROWDOWN, arrowDown);
}

void writeToLcd(const char * line1, const char * line2)
{
  lcd.clear();
  if(line1 != NULL)
  {
    lcd.print(line1);
  }
  if(line2 != NULL)
  {
    lcd.print(line2);
  }
}

//void writeToLcd()
//{
//  lcd.clear();
//  size_t dateOnlyOffset(krakenData.timestamp.length()-12);
//  lcd.printf("%12s[%c%c]", krakenData.timestamp.substring(dateOnlyOffset).c_str(), (krakenData.error? '!' : ' '), (krakenData.parsed? ' ' : '!'));
//  USE_SERIAL.printf("Timestamp shortened: %s\n", krakenData.timestamp.substring(dateOnlyOffset).c_str());
//  lcd.setCursor(0,1);
//  lcd.printf("%.0f %c%.0f %c%.0f", krakenData.lastValue, (uint8_t)ARROWDOWN, krakenData.lowValue, (uint8_t)ARROWUP, krakenData.highValue);
//}

bool HttpGetHelper(String iUrl, const char* iCaCert, DynamicJsonDocument& ioData)
{
  bool error = true;
  WiFiClientSecure client;
  if(iCaCert != NULL)
  {
    client.setCACert(iCaCert);
  }
  HTTPClient http;
  http.setReuse(false);
  USE_SERIAL.printf("[HTTP] Request to: %s\n", iUrl.c_str());
  if(http.begin(client, iUrl)) {
    const int httpCode = http.GET();
    if(httpCode > 0) {
      if(httpCode == HTTP_CODE_OK) {
        USE_SERIAL.printf("[HTTP] Response: 200 OK\n");
        USE_SERIAL.printf("[JSON] Deserializing...\n");
        DeserializationError jsonError = deserializeJson(ioData, http.getString().c_str());
        if (jsonError) {
          USE_SERIAL.printf("[JSON] Error deserializing error: %s\n", jsonError.c_str());
        }
        else
        {
          USE_SERIAL.printf("[JSON] Deserialized!\n");
          serializeJsonPretty(ioData, USE_SERIAL);
          error = false;
        }
      }
      else
      {
        USE_SERIAL.printf("[HTTP] Response: %s\n", http.errorToString(httpCode).c_str());
      }
    } else {
        USE_SERIAL.printf("[HTTP] Error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    USE_SERIAL.printf("[HTTP] Error: unable to setup an HTTPS connection\n");
  }
  return error;
}

struct IpapiData {
  bool   error;
  bool   initialized;
  String lat;
  String lon;
  String city;
  String country;
};

IpapiData ipapiData;

bool getDataFromIpapi() {
  bool error = true;
  if(ipapiData.initialized) {
    USE_SERIAL.printf("[JSON] Skipping as we have data from Ipapi: %s %s %s %s\n", ipapiData.lat.c_str(), ipapiData.lon.c_str(), ipapiData.city.c_str(), ipapiData.country.c_str());
    error = false; 
  }
  else
  {
    error = HttpGetHelper(ipapiUrl, ipapiCaCert, ipapi);
    if(!error)
    {
      ipapiData.lat = String((double)ipapi["latitude"], 3);
      ipapiData.lon = String((double)ipapi["longitude"], 3);
      ipapiData.city = String((const char*)ipapi["city"]);
      ipapiData.country = String((const char*)ipapi["country_name"]);
      USE_SERIAL.printf("[JSON] Decoded data: %s %s %s %s\n", ipapiData.lat.c_str(), ipapiData.lon.c_str(), ipapiData.city.c_str(), ipapiData.country.c_str());
      error = false;
      ipapiData.initialized = true;
    }
  }
  ipapiData.error = error;
  return !error;
}

struct WeatherData {
  bool   error;
  bool   initialized;
  unsigned long dt;
  long timezone;
};

WeatherData weatherData;

bool getDataFromWeather() {
  bool error = true;
  // Build weather url
  String url = String(weatherUrl) + "&lat=" + ipapiData.lat + "&lon=" + ipapiData.lon;
  error = HttpGetHelper(url, weatherCaCert, weather);
  if(!error)
  {
    USE_SERIAL.printf("[JSON] Deserialized!\n");
    weatherData.dt = (unsigned long)weather["dt"];
    weatherData.timezone = (long)weather["timezone"];
    USE_SERIAL.printf("[JSON] Decoded data: %ul %ld\n", weatherData.dt, weatherData.timezone);
    error = false;
    weatherData.initialized = true;
  }
  return error;
}

bool isClockSet = false;
void setupClock() {
  time_t nowSecs = time(nullptr);
  if (isClockSet) {
    USE_SERIAL.print(F("[NTP] Skipping sync\n"));
  }
  else
  {
    configTime(weatherData.timezone, 0, "pool.ntp.org", "time.nist.gov");
    USE_SERIAL.print(F("[NTP] Waiting for NTP time sync: "));
    
    while (nowSecs < 8 * 3600 * 2) {
      delay(500);
      USE_SERIAL.print(F("."));
      yield();
      nowSecs = time(nullptr);
    }
    USE_SERIAL.println();
    //isClockSet = true;
  }
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  getLocalTime( &timeinfo );
  USE_SERIAL.printf("[NTP] Current time (local) considering offset %ul: ", weatherData.timezone);
  USE_SERIAL.print(asctime(&timeinfo));
}

void setupInitializations()
{
  ipapiData.initialized = false;
  weatherData.timezone = 0;
  isClockSet = false;
}

void setup()
{
  USE_SERIAL.begin(115200);
  setupWifi();
  setupLcd();
  setupInitializations();
}

void loop() {
  if((wifiMulti.run() == WL_CONNECTED)) {
    setupClock();
    if(getDataFromIpapi())
    {
      getDataFromWeather();
    }
    delay(refreshIntervalSeconds * 1000);
  }
  else
  {
    setupInitializations();
    USE_SERIAL.printf("[WIFI] Connecting...\n");
    //writeToLcd("Connecting Wifi", "Please wait...");
    delay(1000);
  }
}
