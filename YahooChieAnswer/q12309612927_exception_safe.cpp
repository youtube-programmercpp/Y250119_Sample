#include <tuple>
//ご質問：https://detail.chiebukuro.yahoo.co.jp/qa/question_detail/q12309612927

//同名のファイルを開いてテストデータを書き込む処理を複数回実行する。
//先行する処理でファイルがクローズされていないと後続のオープン時にエラーが発生する
//finally で delete を行えばそこでファイルはクローズされる
//stack semantics を用いればもはや何も考えなくてもファイルは自動的にクローズされる（これがC++/CLI本来の処理形態であると考えたい）
void Test1_LeakByException(System::String^ path, array<System::Byte>^ data, void (*operation)())
{
	auto file = gcnew System::IO::FileStream(path, System::IO::FileMode::Create, System::IO::FileAccess::Write);
	file->Write(data, 0, data->Length);
	operation();
	file->Close();//operation() で例外が投入されると file->Close() は実行されない。ここでファイルハンドル資源のリークが発生する。
}
void Test2_ExceptionSafeByFinallyDelete(System::String^ path, array<System::Byte>^ data, void (*operation)())
{
	auto file = gcnew System::IO::FileStream(path, System::IO::FileMode::Create, System::IO::FileAccess::Write);
	try {
		file->Write(data, 0, data->Length);
		operation();
	}
	finally {
		delete file;//operation() で例外が投入されてもされなくても、それとは関係無く必ず delete file が実行される。
		//delete file が実行されると結果としてIDisposable::Disposeが呼ばれる。
		//この delete はメモリ解放のためではない（メモリ解放はGCが行う）。
		//この delete は資源解放のためである。
	}
}
void Test3_ExceptionSafeByUsingStackSemantics(System::String^ path, array<System::Byte>^ data, void (*operation)())
{
	// 残念ながら、C#用のコードを単純にC++/CLIに置き換えただけのコードばかりが世の中に出回っているように見える。
	// C++/CLIではmanaged オブジェクトに対してスタックセマンティクスを用いることで、極めて簡単に例外安全を実現することが出来る。
	// 【スタックセマンティクスとは】
	// スタック上にオブジェクトを構築しているかのように見せる書き方。
	// gcnew を行っていないようで内部的には行っている。(結局のところGCによるメモリ解放の対象となる)
	// そして、スタック上のオブジェクトがスコープを外れる際それが自動的に破棄されるように、当該オブジェクトのIDisposable::Disposeが呼び出される。
	// 【スタックセマンティクスに関する参考資料】
	// https://learn.microsoft.com/en-us/cpp/dotnet/cpp-stack-semantics-for-reference-types [英語]
	// https://learn.microsoft.com/ja-jp/cpp/dotnet/cpp-stack-semantics-for-reference-types [日本語]
	// 《注意》
	// スタックセマンティクスの説明というよりも、それは簡単な話として早々に片付けてしまい、専らコピーの発動に関して詳しく説明している…かのように見える。
	// コピーに関する説明は、当サンプルで議論の対象としている例外安全（exception safety）においてはこの際重要ではない。
	// よって、この資料を読んで訳が分からなくなったとしても特に問題はない。（と思う）
	System::IO::FileStream file(path, System::IO::FileMode::Create, System::IO::FileAccess::Write);
	file.Write(data, 0, data->Length);
	operation();
}
void OperationNone()
{
	//何もしない
}
void OperationThrowException()
{
	throw gcnew System::Exception(L"例外が投入されました。");
}
int main()
{
	System::Console::Write(
		L"ファイル書き込み処理中に例外が発生した場合にハンドルがリークするかどうかを確かめます。\n"
		L"ハンドルがリークしていれば同名ファイルの書き込み用オープンは失敗します。\n"
		L"ハンドルがリークしていなければ同名ファイルの書き込み用オープンは成功します。\n"
	);
	auto data = gcnew array<System::Byte>{ 'A', 'B', 'C' };
	for (const auto& r : 
	{ std::make_tuple(L"\nTest1: 例外によるハンドルのリーク"             , Test1_LeakByException                   , L"Test1Out.txt")
	, std::make_tuple(L"\nTest2: finallyでdeleteを行うことによる例外安全", Test2_ExceptionSafeByFinallyDelete      , L"Test2Out.txt")
	, std::make_tuple(L"\nTest3: スタックセマンティクスによる例外安全"   , Test3_ExceptionSafeByUsingStackSemantics, L"Test3Out.txt")
	}) {
		System::Console::WriteLine("\nテストを始めます。[Enter] キーを押してください。");
		System::Console::ReadKey();
		System::Console::WriteLine(gcnew System::String(std::get<0>(r)));
		for (int i = 0; i < 4; ++i) {
			try {
				System::Console::WriteLine(L"{0} 回目", i + 1);
				std::get<1>(r)(gcnew System::String(std::get<2>(r)), data, i & 1 ? OperationThrowException : OperationNone);
			}
			catch (System::Exception^ e) {
				System::Console::Error->WriteLine(e->Message);
			}
		}
	}
}
/*
https://detail.chiebukuro.yahoo.co.jp/qa/question_detail/q12309612927
pat********さん

2025/1/18 23:02

1回答

C++/CLIで、ファイルアクセス中に発生した例外をキャッチし、確実にファイルをクローズするサンプルプログラムを作成して下さい。
下のプログラムでは、例外の発生によってはファイルがクローズされずに終わる可能性があります。
tss1 = gcnew IO::FileStream(ts1, IO::FileMode::Open, IO::FileAccess::Read);
binBuffer = gcnew array<byte>(tss1->Length);

tss1->Read(binBuffer, 0, (int)(tss1->Length));
tss1->Close();

C言語関連・16閲覧
*/
