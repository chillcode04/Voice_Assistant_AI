# Device for recording and summarizing meetings
An ESP32-based device that records meetings, creates AI-generated summaries using the Gemini API, and updates Google Sheets. The system employs FreeRTOS for multithreading to accelerate audio upload, while a Google Apps Script web application provides an interface for entering and submitting report information.

## Components
- ESP32-C6  
- INMP441 
- Module SD Card  
- Oled 0.96'
## System Architecture Diagram 
![image alt](https://github.com/chillcode04/Voice_Assistant_AI/blob/eede9baf7a6a976ab35102ece66296525fb5b1ef/Screenshot%202025-09-09%20223114.png)
## A web page for entering information to update a Google Sheet
![image alt](https://github.com/chillcode04/Voice_Assistant_AI/blob/eede9baf7a6a976ab35102ece66296525fb5b1ef/Screenshot%202025-09-09%20223503.png)
## Device circuit design
![image alt](https://github.com/chillcode04/Voice_Assistant_AI/blob/eede9baf7a6a976ab35102ece66296525fb5b1ef/Screenshot%202025-09-09%20223509.png)
## The meeting summary results are updated in the corresponding position on the Google Sheet
![image alt](https://github.com/chillcode04/Voice_Assistant_AI/blob/eede9baf7a6a976ab35102ece66296525fb5b1ef/Screenshot%202025-09-09%20223539.png)
