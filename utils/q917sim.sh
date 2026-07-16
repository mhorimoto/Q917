#!/bin/bash

# ==========================================
# UECS Simulator Remote Control Tool
# ==========================================

# 宛先設定（環境に合わせて変更してください）
TARGET_PORT="16528"

# ヘルプ表示
show_help() {
    echo "Usage: $(basename "$0") Q917_IP_Address <command> [arguments]"
    echo ""
    echo "Commands:"
    echo "  ver                     バージョン情報を取得します"
    echo "  dump [block]            EEPROMのメモリダンプを表示します (block: 0-3)"
    echo "  reset                   マイコンをソフトリセットします"
    echo "  set [base] [char] [room] [region] [order] [priority] [interval] [name]"
    echo "                          指定したセンサのUECSパラメータを一括書き換えします"
    echo "                          例: $(basename "$0") set 10 T 02 01 300 10 15 InAirTemp"
    echo "  raw [string]            任意のコマンド文字列を直接送信します (例: raw S01105)"
    exit 1
}

# 送信用関数
send_cmd() {
    local cmd="$1"
    local timeout="$2"
    
    if [ "$timeout" = "nowait" ]; then
        echo -n "$cmd" | socat - UDP4:"${TARGET_IP}:${TARGET_PORT}"
    else
        echo -n "$cmd" | socat -t 2 - UDP4:"${TARGET_IP}:${TARGET_PORT}"
    fi
}

# 引数チェック
if [ $# -lt 1 ]; then
    show_help
fi

TARGET_IP="$1"
SUB_CMD="$2"
shift

case "$SUB_CMD" in
    ver)
        send_cmd "V"
        echo "" # 改行用
        ;;
        
    dump)
        BLOCK="${2:-0}" # 引数がない場合はデフォルトで 0
        send_cmd "D${BLOCK}"
        ;;
        
    reset)
        echo "Resetting UECS Simulator..."
        send_cmd "R77" "nowait"
        ;;
        
    set)
        if [ $# -lt 8 ]; then
            echo "Error: 'set' command requires 8 arguments."
            echo "Format: set <base> <char> <room> <region> <order> <priority> <interval> <name>"
            exit 1
        fi
        shift
        # 各引数のフォーマット整形
        BASE=$(printf "%02X" "0x$1" 2>/dev/null || printf "%02s" "$1")
        CHAR="${2:0:1}" # 1文字目のみ使用
        ROOM=$(printf "%02X" "$3")
        REGION=$(printf "%02X" "$4")
        ORDER=$(printf "%04X" "$5") # 2バイト(4桁)に変換
        PRIO=$(printf "%02X" "$6")
        INTERVAL=$(printf "%02X" "$7")
        NAME="$8"
        
        # Wコマンド文字列の合成
        W_CMD="W${BASE}${CHAR}${ROOM}${REGION}${ORDER}${PRIO}${INTERVAL}${NAME}"
        
        echo "Sending: $W_CMD"
        send_cmd "$W_CMD"
        echo ""
        ;;
        
    raw)
        if [ -z "$2" ]; then
            echo "Error: Please specify the raw string."
            exit 1
        fi
        send_cmd "$2"
        echo ""
        ;;
        
    *)
        show_help
        ;;
esac
