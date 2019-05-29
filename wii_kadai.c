/*************************************************
sdl_wii.c
Programmed by S. Nishide (2019,5)

コンパイルオプション：-lSDL2 -lSDL2_gfx -lcwiimote -D_ENABLE_TILT -D_ENABLE_FORCE -L/usr/lib
備考：実行する時は./実行ファイル名 WiiリモコンのMACアドレス

*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>	// SDLを用いるために必要なヘッダファイル
#include <SDL2/SDL2_gfxPrimitives.h>	// 描画関係のヘッダファイル

#include "wiimote.h"	// Wiiリモコンを用いるために必要なヘッダファイル
#include "wiimote_api.h"	// Wiiリモコンを用いるために必要なヘッダファイル

int null_thread(void *);

// メイン関数
int main(int argc, char* argv[]) {

  SDL_Window* window; // ウィンドウデータを格納する構造体
  SDL_Renderer* renderer; // 2Dレンダリングコンテキスト(描画設定)を格納する構造体

  SDL_Rect ground = {0, 400, 512, 112}; // 地面
  SDL_Rect sky = {10, 0, 492, 400};     // 空
  SDL_Rect wall1 = {0, 0, 10, 400};     // 左の壁
  SDL_Rect wall2 = {502, 0, 10, 400};   // 右の壁

  SDL_Rect chara = {100, 350, 20, 50};  // キャラの位置情報
  SDL_Rect chara_prev = chara;

  SDL_Thread *thread;
  SDL_atomic_t atm;
  
  int jumpflag = 0;
  int fallflag = 0;
  int syagamiflag1 = 1 ,syagamiflag2 = 0;
  int spincount = 0;
  int spinflag = 0, spinfallflag = 0, spinjumpflag = 0;
  int red = 0, blue = 0;
  int redball = 0, blueball = 0;
  //int redtamazahyox = 0, redtamazahyoy = 0;
  //int bluetamazahyox = 0, bluetamazahyoy = 0;
  int tamazahyox = 0, tamazahyoy = 0;
  /**************************************/
  /* Wiiリモコン関連の初期化（ここから） */
  /**************************************/

  // Wiiリモコンを用いるための構造体を宣言（初期化）
  wiimote_t wiimote = WIIMOTE_INIT;	// Wiiリモコンの状態格納用
  wiimote_report_t report = WIIMOTE_REPORT_INIT;	// レポートタイプ用
  
  // Wiiリモコンのスピーカで鳴らす音のデータ
  uint8_t sample[20] = {
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c
  };

  // ***** Wiiリモコン処理 *****
  if (argc < 2) {	// Wiiリモコン識別情報がコマンド引数で与えられなければ
    printf("Designate the wiimote ID to the application.\n");
    exit(1);
  }
  
  // Wiiリモコンの接続（１つのみ）
  if (wiimote_connect(&wiimote, argv[1]) < 0) {	// コマンド引数に指定したWiiリモコン識別情報を渡して接続
    printf("unable to open wiimote: %s\n", wiimote_get_error());
    exit(1);
  }
  wiimote.led.one  = 1;	// WiiリモコンのLEDの一番左を点灯させる（接続を知らせるために）
  // wiimote.led.four  = 1;	// WiiリモコンのLEDの一番右を点灯させる場合
  
  // Wiiリモコンスピーカの初期化
  wiimote_speaker_init(&wiimote, WIIMOTE_FMT_S4, WIIMOTE_FREQ_44800HZ);

  // センサからのデータを受け付けるモードに変更
  wiimote.mode.acc = 1;

  /**************************************/
  /* Wiiリモコン関連の初期化（ここまで） */
  /**************************************/

  
  /**************************************/
  /* SDL関連の初期化（ここから）         */
  /**************************************/
  
  // SDL初期化
  if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    printf("failed to initialize SDL.\n");
    exit(-1);
  }

  // ウィンドウ生成・表示
  window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, 0);
  if(window == NULL){
    printf("Failed to create window.\n");
    exit(-1);
  }

  // 描画処理
  renderer = SDL_CreateRenderer(window, -1, 0); // 生成したウィンドウに対してレンダリングコンテキスト（RC）を生成
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0); // 生成したRCに描画色として白を設定
  SDL_RenderClear(renderer); // 生成したRCを白でクリア＝塗りつぶす（ただし，メモリに描画データを反映させただけなので，画面には表示されない）

  // 初期画面
  stringColor(renderer, 0, 0, "Press buttons 1 and 2 on the wiimote now to connect.\n", 0xffffffff);  
  SDL_RenderPresent(renderer); // 描画データを表示
  
  /**************************************/
  /* SDL関連の初期化（ここまで）         */
  /**************************************/

  // 地面・壁・空の描画
  SDL_SetRenderDrawColor(renderer, 153, 76, 0, 0);
  SDL_RenderFillRect(renderer, &ground);           // 地面を描画
  SDL_RenderFillRect(renderer, &wall1);            // 左の壁を描画 
  SDL_RenderFillRect(renderer, &wall2);            // 右の壁を描画
  SDL_SetRenderDrawColor(renderer, 200, 200, 255, 0);
  SDL_RenderFillRect(renderer, &sky);              // 空の描画

  // キャラの描画
  SDL_SetRenderDrawColor(renderer, 255, 150, 150, 0);
  SDL_RenderFillRect(renderer, &chara);              // キャラの描画
  SDL_RenderPresent(renderer); // 描画データを表示

  // 描画画面が暗くならないようにイベント処理をマルチスレッドで入れる
  SDL_AtomicSet(&atm, 1);
  thread = SDL_CreateThread(null_thread, "none", &atm);
  SDL_DetachThread(thread);
  
  /**************************************/
  /* 処理開始                            */
  /**************************************/

  // Wiiリモコンがオープン（接続状態）であればループ
  while (wiimote_is_open(&wiimote)) {
    
    // Wiiリモコンの状態を取得・更新する
    if (wiimote_update(&wiimote) < 0) {
      wiimote_disconnect(&wiimote);
      break;
    }

    // ***** Wiiのキー（ボタン）ごとに処理 *****
    // HOMEボタンが押された時(終了処理)
    if (wiimote.keys.home) {
      SDL_AtomicSet(&atm, 0);
      wiimote_speaker_free(&wiimote);	// Wiiリモコンのスピーカを解放
      wiimote_disconnect(&wiimote);	// Wiiリモコンとの接続を解除
    }

    /***********************************/
    /* 以下に処理を記述していく         */
    /***********************************/
    
    /*	加速度の測定		*/
/*printf("FORCE x=%.3f y=%.3f z=%.3f (sum=%.3f)\n", 
	   wiimote.force.x,
	   wiimote.force.y,
	   wiimote.force.z,
	   sqrt(wiimote.force.x*wiimote.force.x+wiimote.force.y*wiimote.force.y+wiimote.force.z*wiimote.force.z));*/

    
	if(wiimote.keys.down){//十字キー→
		chara.x +=2;
	}else if(wiimote.keys.up){//十字キー←
		chara.x -=2;
	}
	
	//しゃがみ処理
	if(wiimote.keys.left){//左ボタンが押された場合
		if(syagamiflag1 == 1){
			chara.y += 25;
			chara.h = 25;
			syagamiflag1 = 0;
			syagamiflag2 = 1;
		}
	}else{
		if(syagamiflag2 == 1){//しゃがみから元に戻った場合
			chara.y -=25;
			chara.h = 50;
			syagamiflag2 = 0;
			syagamiflag1 = 1;
		}
	}
	//ダッシュ処理
	if(wiimote.keys.one){//1ボタンが押された
		if(wiimote.keys.down){//十字キー→
			chara.x +=4;
		}
		if(wiimote.keys.up){//十字キー←
			chara.x -=4;
		}
		if(red == 1 && redball == 0){//赤色になってる場合
			redball = 1;
			//押された時の正面部分の座標を記憶
			tamazahyox = chara.x+20;
			tamazahyoy = chara.y+25;
		}else if(blue == 1 && blueball == 0){//青色になっている場合
			blueball = 1;
			//押された時の正面部分の座標を記憶
			tamazahyox = chara.x+20;
			tamazahyoy = chara.y+25;
		}
	}
	//ジャンプ処理
	if(wiimote.keys.two && jumpflag == 0 && spinflag == 0){
		jumpflag = 1;//ジャンプ用のフラグを立てる
	}
	if(jumpflag == 1 && fallflag == 0){
		chara.y -=1;
	}
	if(chara.y <= 200 && jumpflag == 1){
		fallflag = 1;
	}
	if(fallflag == 1){
		chara.y +=1;
	}
	if(syagamiflag2 == 1){
		if(chara.y >= 375 && fallflag == 1){
			chara.y = 375;
			chara.h = 25;
			fallflag = 0;
			jumpflag = 0;
		}
	}else{
		 if(chara.y >= 350 && fallflag == 1){
			chara.y = 350;
			fallflag = 0;
			jumpflag = 0;
			chara.h = 50;
		}
	}

	//スピンジャンプの処理
	if(2 <= sqrt(wiimote.force.x*wiimote.force.x+wiimote.force.y*wiimote.force.y+wiimote.force.z*wiimote.force.z)){
		spincount++;
		if(spincount >= 5){
			spinflag = 1;
			spincount = 0;
		}
	}else{
		spincount = 0;
	}
	if(spinflag == 1 && jumpflag == 0 && spinjumpflag == 0){
		spinjumpflag = 1;
		chara.w = 10;
	}
	if(spinjumpflag == 1 && spinfallflag ==0){
		chara.y -=1;
	}
	if(chara.y <= 200 && spinjumpflag == 1){
		spinfallflag = 1;
	}
	if(spinfallflag == 1){
		chara.y +=1;
	}
	if(syagamiflag2 == 1){
		if(chara.y >= 375 && spinfallflag == 1){
			chara.y = 375;
			chara.h = 25;
			spinfallflag = 0;
			spinjumpflag = 0;
			chara.w = 20;
			spinflag = 0;
		}
	}else{
		 if(chara.y >= 350 && spinfallflag == 1){
			chara.y = 350;
			spinfallflag = 0;
			spinjumpflag = 0;
			chara.h = 50;
			chara.w = 20;
			spinflag = 0;
		}
	}	

	//色変化の処理
	if(chara.x <= 30 && chara.y + 50 >= 380){
		red = 1;
		blue = 0;
	}else if(chara.x+20 >= 480 && chara.y + 50 >= 380){
		red = 0;
		blue = 1;
	}
	
	//玉発射の処理
	if(redball == 1){
		tamazahyox = tamazahyox + 3;
		if(tamazahyox+2 >= 502){
			redball = 0;
		}
	}
	if(blueball == 1){
		tamazahyox = tamazahyox + 3;
		if(tamazahyox+2 >= 502){
			blueball = 0;
		}
	}
	
    /***********************************/
    /* 処理の記述ここまで               */
    /***********************************/

    // *****図形描画処理*****
	if(red == 1){
  		SDL_SetRenderDrawColor(renderer, 153, 76, 0, 0);
  		SDL_RenderFillRect(renderer, &ground);           // 地面を描画
  		SDL_RenderFillRect(renderer, &wall1);            // 左の壁を描画 
  		SDL_RenderFillRect(renderer, &wall2);            // 右の壁を描画
  		SDL_SetRenderDrawColor(renderer, 200, 200, 255, 0);
  		SDL_RenderFillRect(renderer, &sky);              // 空の描画
    	filledCircleColor(renderer, 25, 380, 5, 0xff0000ff); // 左に赤丸アイテム
    	filledCircleColor(renderer, 485, 380, 5, 0xffff0000); // 右に青丸アイテム
    	if(redball == 1){
    		filledCircleColor(renderer, tamazahyox, tamazahyoy, 2, 0xff0000ff); // 赤玉の描画
    	}
    	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
    	SDL_RenderFillRect(renderer, &chara);              // キャラの描画
    
    	SDL_RenderPresent(renderer);	// 描画データを表示

    	chara_prev = chara;
    
	}else if(blue == 1){
  		SDL_SetRenderDrawColor(renderer, 153, 76, 0, 0);
  		SDL_RenderFillRect(renderer, &ground);           // 地面を描画
  		SDL_RenderFillRect(renderer, &wall1);            // 左の壁を描画 
  		SDL_RenderFillRect(renderer, &wall2);            // 右の壁を描画
  		SDL_SetRenderDrawColor(renderer, 200, 200, 255, 0);
  		SDL_RenderFillRect(renderer, &sky);              // 空の描画
    	filledCircleColor(renderer, 25, 380, 5, 0xff0000ff); // 左に赤丸アイテム
    	filledCircleColor(renderer, 485, 380, 5, 0xffff0000); // 右に青丸アイテム
    	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 0);
    	SDL_RenderFillRect(renderer, &chara);              // キャラの描画
    	if(blueball == 1){
    		filledCircleColor(renderer, tamazahyox, tamazahyoy, 2, 0xffff0000); // 赤玉の描画
    	}
    	SDL_RenderPresent(renderer);	// 描画データを表示

    	chara_prev = chara;
    }else{
  		SDL_SetRenderDrawColor(renderer, 153, 76, 0, 0);
  		SDL_RenderFillRect(renderer, &ground);           // 地面を描画
  		SDL_RenderFillRect(renderer, &wall1);            // 左の壁を描画 
  		SDL_RenderFillRect(renderer, &wall2);            // 右の壁を描画
  		SDL_SetRenderDrawColor(renderer, 200, 200, 255, 0);
  		SDL_RenderFillRect(renderer, &sky);              // 空の描画
    	filledCircleColor(renderer, 25, 380, 5, 0xff0000ff); // 左に赤丸アイテム
    	filledCircleColor(renderer, 485, 380, 5, 0xffff0000); // 右に青丸アイテム
    	SDL_SetRenderDrawColor(renderer, 255, 150, 150, 0);
    	SDL_RenderFillRect(renderer, &chara);              // キャラの描画
    
    	SDL_RenderPresent(renderer);	// 描画データを表示

    	chara_prev = chara;
    }
  }

  SDL_Delay(300);
  
  // 終了処理
  SDL_DestroyRenderer(renderer); // RCの破棄（解放）
  SDL_DestroyWindow(window); // 生成したウィンドウの破棄（消去）
  SDL_AtomicDecRef(&atm);
  SDL_Quit();
  return 0;
}

int null_thread(void *atm){

  SDL_Event event;

  while(1){
    if (SDL_PollEvent(&event)){
      SDL_Delay(100);
    }
    if (SDL_AtomicGet(atm) == 0 ){
      break;
    }
  }
  
}
