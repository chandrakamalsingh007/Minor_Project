# Minor_Project : Face-recognition based door locking system with OTP verification (ESP32 + Django)

> A **multi-layered smart door security system** combining **anti-spoofing face recognition** (Fourier Transform + CNN), **two-factor authentication via OTP** (Telegram Bot), and **cloud-controlled hardware** using ESP32-CAM and Django backend.

---

# 📸 Live Demo & Preview

🔗 **[Anti Spoofing](Demo/anti_spoofing.mp4)** 
🔗 **[Face Recognition Sample](Demo/facerecog_sample.mp4)** 

---

# 🚀 Key Features

| Feature | Description |
|---------|-------------|
| 🛡️ **Anti-Spoofing** | Fourier Transform + CNN detects printed photos, video replays, and masks (97.8% accuracy) |
| 👤 **Face Recognition** | DLIB ResNet-34 model with 128-D embeddings trained on 3M images (99% accuracy) |
| 🔐 **Two-Factor Authentication** | Face matched → OTP sent via Telegram → OTP entered on keypad |
| ☁️ **Cloud Backend** | Django REST API for user management, remote access, and centralized logging |
| 📡 **Hardware Integration** | ESP32-CAM captures image → Arduino controls solenoid lock via relay |
| 📱 **Real-Time UI** | Web interface for face registration + Telegram OTP delivery |
| 📊 **Centralized Logging** | All access attempts logged in database with timestamps and images |

---

# 🧠 System Architecture (5-Layer Model)

```text
┌──────────────────────────────────────────────────────┐
│ 1. Image Acquisition Layer (ESP32-CAM)              │
│ → Captures 80×80 RGB face image                     │
└──────────────────────────────────────────────────────┘
                         ↓
┌──────────────────────────────────────────────────────┐
│ 2. Anti-Spoofing Layer (Fourier + CNN)              │
│ → Spatial Branch (MiniFASNet)                       │
│ → Frequency Branch (FFT + FTGenerator)              │
│ → Fuse outputs → 97.8% accuracy                     │
└──────────────────────────────────────────────────────┘
                         ↓
┌──────────────────────────────────────────────────────┐
│ 3. Face Recognition Layer (DLIB ResNet-34)          │
│ → Detect 68 landmarks                               │
│ → Align & normalize face                            │
│ → Compute 128-D embedding                           │
│ → Compare with registered faces                     │
└──────────────────────────────────────────────────────┘
                         ↓
┌──────────────────────────────────────────────────────┐
│ 4. Communication Layer (HTTP + Django API)          │
│ → Send face encoding to backend                     │
│ → Verify identity                                   │
│ → Generate OTP via Telegram Bot                     │
└──────────────────────────────────────────────────────┘
                         ↓
┌──────────────────────────────────────────────────────┐
│ 5. Hardware Control Layer (Arduino + Relay)         │
│ → Receive OTP from keypad                           │
│ → Validate OTP                                      │
│ → Activate solenoid lock                            │
└──────────────────────────────────────────────────────┘
````

📷 *See `assets/system_architecture.png` for detailed diagram.*

---

# 🛠️ Tech Stack

| Layer                | Technology                                          | Purpose                            |
| -------------------- | --------------------------------------------------- | ---------------------------------- |
| **Backend**          | Django + Django REST Framework                      | REST API, user management, logging |
| **Face Recognition** | DLIB, OpenCV, ResNet-34                             | 128-D face embeddings              |
| **Anti-Spoofing**    | PyTorch, NumPy, Fourier Transform                   | Detects spoof attacks              |
| **Hardware**         | ESP32-CAM, Arduino Uno, 16×2 LCD, 4×4 Keypad, Relay | Image capture & lock control       |
| **Communication**    | HTTP, Telegram Bot API                              | OTP delivery, cloud sync           |
| **Database**         | SQLite (dev) / PostgreSQL (prod)                    | Face encodings, logs, users        |
| **Frontend**         | HTML, CSS, Bootstrap                                | Face registration UI               |

---

# 📊 Performance Metrics

| Model                             | Accuracy | FPR    | Speed | FLOPs |
| --------------------------------- | -------- | ------ | ----- | ----- |
| Anti-Spoof (Fourier + CNN)        | 97.8%    | 0.001% | 20ms  | 84M   |
| Face Recognition (DLIB ResNet-34) | 99.0%    | 0.6%   | 35ms  | 500M  |

---

# 📦 Installation & Setup

## ✅ Prerequisites

* Python 3.8+
* Git
* Arduino IDE
* Telegram Bot Token

---

## 1️⃣ Clone the Repository

```bash
git clone https://github.com/yourusername/Face-Door-Lock-OTP.git
cd Face-Door-Lock-OTP
```

---

## 2️⃣ Set Up Python Virtual Environment

### Windows

```bash
python -m venv venv
venv\Scripts\activate
```

### Linux / macOS

```bash
python3 -m venv venv
source venv/bin/activate
```

---

## 3️⃣ Install Dependencies

```bash
pip install -r requirements.txt
```

---

## 4️⃣ Configure Environment Variables

Create a `.env` file in the root directory:

```env
# Django settings
SECRET_KEY=your_django_secret_key
DEBUG=True

# Telegram Bot (create via @BotFather)
TELEGRAM_BOT_TOKEN=your_telegram_bot_token
TELEGRAM_CHAT_ID=your_telegram_chat_id

# Database (optional for production)
DATABASE_URL=postgresql://user:pass@localhost/dbname
```

---

## 5️⃣ Run Django Backend

```bash
cd backend

python manage.py migrate
python manage.py createsuperuser
python manage.py runserver
```

---

## 6️⃣ Flash ESP32-CAM

1. Open `hardware/esp32_cam.ino` in Arduino IDE
2. Update Wi-Fi credentials and server URL:

```cpp
const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";
const char* serverUrl = "http://127.0.0.1:8000/api/verify-face/";
```

3. Select board: **AI Thinker ESP32-CAM**
4. Upload firmware to ESP32-CAM

---

## 7️⃣ Test the System

### Web Interface

Visit:

```text
http://127.0.0.1:8000/register/
```

Register a face using the web dashboard.

### Hardware Workflow

```text
ESP32-CAM → Capture Face → Verify Face
→ Send OTP via Telegram → Enter OTP on Keypad
→ Unlock Door
```

---

# 📁 Project Structure

```text
Face-Door-Lock-OTP/
│
├── README.md
├── LICENSE
│
├── backend/
│   ├── manage.py
│   │
│   ├── doorlock/
│   │   ├── models.py
│   │   ├── views.py
│   │   ├── urls.py
│   │   ├── serializers.py
│   │   └── admin.py
│   │
│   ├── anti_spoof/
│   │   ├── model.pth
│   │   ├── preprocess.py
│   │   └── predict.py
│   │
│   └── face_rec/
│       ├── encodings.pickle
│       └── recognizer.py
│
├── hardware/
│   ├── esp32_cam.ino
│   ├── arduino_lock.ino
│   └── circuit_diagram.png
│
├── ui/
│   └── templates/
│       ├── register.html
│       └── dashboard.html
│
├── assets/
│   ├── face_recognition.png
│   ├── anti_spoof.png
│   ├── telegram_otp.png
│   ├── demo_video.mp4
│   └── system_architecture.png
│
└── tests/
    ├── test_anti_spoof.py
    ├── test_face_rec.py
    └── test_otp.py
```

---

# 📋 API Endpoints

| Method | Endpoint              | Description                              |
| ------ | --------------------- | ---------------------------------------- |
| `POST` | `/api/verify-face/`   | Receive face image → verify → return OTP |
| `POST` | `/api/verify-otp/`    | Validate OTP → unlock door               |
| `POST` | `/api/register-face/` | Register new face with name              |
| `GET`  | `/api/logs/`          | Fetch access logs                        |
| `POST` | `/api/send-otp/`      | Manually trigger OTP to Telegram         |

---

# 🧪 Testing

Run unit tests:

```bash
cd backend
python manage.py test
```

---

# 👥 Team

| Name                | Role                       | GitHub        |
| ------------------- | -------------------------- | ------------- |
| Aagaman K.C.        | Backend & Anti-Spoof Model | @aagaman      |
| Ajay Panta          | Face Recognition & DLIB    | @ajaypanta    |
| Chandra Kamal Singh | Hardware Integration       | @chandrakamal |
| Gaurav Bhujel       | UI & Telegram Integration  | @gauravbhujel |

---

# 📜 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

# 🙏 Acknowledgments

* **D. S. Rajavel et al.** – Base paper for OTP integration
* **Dimas Ricky Saputra** – ESP32-CAM face recognition reference
* **Burruel-Zazueta et al.** – Deep learning techniques for biometric locks
* **zhuyingSeu** – Anti-spoofing model architecture

---

# 📧 Contact

* **GitHub:** [@yourusername](https://github.com/yourusername)
* **Email:** [your.email@example.com](mailto:your.email@example.com)
* **LinkedIn:** Your LinkedIn Profile

---

# ⭐ Future Improvements

* Mobile app integration
* MQTT-based real-time communication
* Face recognition using ArcFace
* Liveness detection with eye blink tracking
* Cloud deployment using Docker + Kubernetes
* RFID + Face dual authentication

---

# 🌟 Support

If you found this project useful, consider giving it a ⭐ on GitHub!

```
```

