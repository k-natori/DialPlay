# DialPlay

M5Stack M5Dial で Spotify の再生をコントロールするファームウェアです。

---

## 目次 / Table of Contents

- [日本語](#日本語)
- [English](#english)

---

# 日本語

## 概要

M5Dial のダイヤルとタッチスクリーンで Spotify の再生操作を行うデバイスです。

| 操作 | 機能 |
|------|------|
| ダイヤルを回す | 音量調整 |
| 画面中央をタップ | 再生 / 一時停止 |
| 画面左をタップ | 前のトラックへ |
| 画面右をタップ | 次のトラックへ |
| ボタンを押す | 再生デバイスの切替画面 |
| 起動時にボタンを押したまま | WiFi・認証のリセット |

> Spotify Premium アカウントが必要です。

---

## 必要なもの

- [M5Stack M5Dial](https://docs.m5stack.com/en/core/M5Dial)
- [PlatformIO](https://platformio.org/) をインストールした開発環境
- Spotify Premium アカウント
- Spotify Developer アカウント（後述）

---

## セットアップ

### 1. Spotify Developer アプリの登録

**ソースコードに含まれている Client ID はリポジトリ作者のものです。できる限りご自身のアプリケーションを登録してご利用ください。**

1. [Spotify Developer Dashboard](https://developer.spotify.com/dashboard) にアクセスしてアプリを作成
2. **Redirect URIs** に以下を追加
   - `https://<GitHubユーザー名>.github.io/DialPlay/callback.html`（後述の手順で準備）
   - またはご自身で用意した HTTPS エンドポイント
3. Client ID をメモしておく

### 2. 認証コールバックページの準備

Spotify の認証完了後、ブラウザを M5Dial のローカルアドレスへ転送するための中継ページが必要です。

このリポジトリの `docs/callback.html` を GitHub Pages で公開する方法が最も簡単です。

1. このリポジトリを GitHub にプッシュ
2. リポジトリの **Settings → Pages** を開く
3. Source を `main` ブランチ・`/docs` フォルダに設定して保存
4. `https://<GitHubユーザー名>.github.io/DialPlay/callback.html` が公開される

### 3. ソースコードの書き換え

`src/SPConstant.cpp` の Client ID を自分のものに変更します。

```cpp
String clientID = "ここに自分の Client ID を入力";
```

`src/SPClient.cpp` の redirect URI をコールバックページの URL に変更します。

```cpp
#define authRedirectURL "https://<GitHubユーザー名>.github.io/DialPlay/callback.html"
```

### 4. ビルドと書き込み

```bash
pio run --target upload
```

---

## 初回セットアップ（デバイス上での操作）

### ステップ 1：WiFi の設定

1. 起動すると M5Dial が QR コードを表示します
2. スマートフォンで QR コードを読み取り、`DialPlay` という WiFi に接続します（パスワード：`DialPlay`）
3. キャプティブポータルが自動で開くか、QR コードを再度読み取ってフォームを開きます
4. 接続したい WiFi の SSID とパスワードを入力して送信します

### ステップ 2：Spotify 認証

1. WiFi 接続後、M5Dial に QR コードが表示されます
2. QR コードを読み取るか、表示された URL にアクセスします
3. Spotify のログイン画面が開くので、アカウントでログインして許可します
4. 認証完了後、自動的に再生画面に移行します

以降は電源を入れるだけで自動的に接続されます。

---

## ライセンス

MIT License

---

# English

## Overview

A firmware for M5Stack M5Dial that controls Spotify playback using its rotary dial and touchscreen.

| Control | Function |
|---------|----------|
| Rotate dial | Adjust volume |
| Tap center of screen | Play / Pause |
| Tap left of screen | Previous track |
| Tap right of screen | Next track |
| Press button | Switch playback device |
| Hold button at startup | Reset WiFi and authentication |

> Requires a Spotify Premium account.

---

## Requirements

- [M5Stack M5Dial](https://docs.m5stack.com/en/core/M5Dial)
- Development environment with [PlatformIO](https://platformio.org/) installed
- Spotify Premium account
- Spotify Developer account (see below)

---

## Setup

### 1. Register a Spotify Developer App

**The Client ID included in this source code belongs to the repository author. Please register and use your own Spotify application whenever possible.**

1. Go to [Spotify Developer Dashboard](https://developer.spotify.com/dashboard) and create an app
2. Add the following to **Redirect URIs**:
   - `https://<YourGitHubUsername>.github.io/DialPlay/callback.html` (set up in the next step)
   - Or any HTTPS endpoint you have prepared
3. Note down your Client ID

### 2. Prepare the Auth Callback Page

After Spotify authentication, the browser needs to be forwarded to M5Dial's local address. A relay page is required for this.

The easiest way is to publish `docs/callback.html` from this repository via GitHub Pages.

1. Push this repository to GitHub
2. Open **Settings → Pages** in your repository
3. Set Source to the `main` branch and `/docs` folder, then save
4. `https://<YourGitHubUsername>.github.io/DialPlay/callback.html` will be published

### 3. Update the Source Code

Edit `src/SPConstant.cpp` to use your own Client ID:

```cpp
String clientID = "your_client_id_here";
```

Edit `src/SPClient.cpp` to use your callback page URL:

```cpp
#define authRedirectURL "https://<YourGitHubUsername>.github.io/DialPlay/callback.html"
```

### 4. Build and Upload

```bash
pio run --target upload
```

---

## First-Time Setup (on the device)

### Step 1: WiFi Configuration

1. On startup, M5Dial displays a QR code
2. Scan the QR code with your smartphone and connect to the `DialPlay` WiFi network (password: `DialPlay`)
3. A captive portal will open automatically, or scan the QR code again to open the form
4. Enter the SSID and password of your home WiFi and submit

### Step 2: Spotify Authentication

1. After connecting to WiFi, M5Dial displays another QR code
2. Scan the QR code or navigate to the displayed URL
3. Log in with your Spotify account and grant permission
4. After authentication, the playback screen appears automatically

From the next startup onward, the device connects automatically.

---

## License

MIT License
