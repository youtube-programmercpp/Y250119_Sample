#include <tuple>
//������Fhttps://detail.chiebukuro.yahoo.co.jp/qa/question_detail/q12309612927

//�����̃t�@�C�����J���ăe�X�g�f�[�^���������ޏ����𕡐�����s����B
//��s���鏈���Ńt�@�C�����N���[�Y����Ă��Ȃ��ƌ㑱�̃I�[�v�����ɃG���[����������
//finally �� delete ���s���΂����Ńt�@�C���̓N���[�Y�����
//stack semantics ��p����΂��͂≽���l���Ȃ��Ă��t�@�C���͎����I�ɃN���[�Y�����i���ꂪC++/CLI�{���̏����`�Ԃł���ƍl�������j
void Test1_LeakByException(System::String^ path, array<System::Byte>^ data, void (*operation)())
{
	auto file = gcnew System::IO::FileStream(path, System::IO::FileMode::Create, System::IO::FileAccess::Write);
	file->Write(data, 0, data->Length);
	operation();
	file->Close();//operation() �ŗ�O������������ file->Close() �͎��s����Ȃ��B�����Ńt�@�C���n���h�������̃��[�N����������B
}
void Test2_ExceptionSafeByFinallyDelete(System::String^ path, array<System::Byte>^ data, void (*operation)())
{
	auto file = gcnew System::IO::FileStream(path, System::IO::FileMode::Create, System::IO::FileAccess::Write);
	try {
		file->Write(data, 0, data->Length);
		operation();
	}
	finally {
		delete file;//operation() �ŗ�O����������Ă�����Ȃ��Ă��A����Ƃ͊֌W�����K�� delete file �����s�����B
		//delete file �����s�����ƌ��ʂƂ���IDisposable::Dispose���Ă΂��B
		//���� delete �̓���������̂��߂ł͂Ȃ��i�����������GC���s���j�B
		//���� delete �͎�������̂��߂ł���B
	}
}
void Test3_ExceptionSafeByUsingStackSemantics(System::String^ path, array<System::Byte>^ data, void (*operation)())
{
	// �c�O�Ȃ���AC#�p�̃R�[�h��P����C++/CLI�ɒu�������������̃R�[�h�΂��肪���̒��ɏo����Ă���悤�Ɍ�����B
	// C++/CLI�ł�managed �I�u�W�F�N�g�ɑ΂��ăX�^�b�N�Z�}���e�B�N�X��p���邱�ƂŁA�ɂ߂ĊȒP�ɗ�O���S���������邱�Ƃ��o����B
	// �y�X�^�b�N�Z�}���e�B�N�X�Ƃ́z
	// �X�^�b�N��ɃI�u�W�F�N�g���\�z���Ă��邩�̂悤�Ɍ����鏑�����B
	// gcnew ���s���Ă��Ȃ��悤�œ����I�ɂ͍s���Ă���B(���ǂ̂Ƃ���GC�ɂ�郁��������̑ΏۂƂȂ�)
	// �����āA�X�^�b�N��̃I�u�W�F�N�g���X�R�[�v���O���ۂ��ꂪ�����I�ɔj�������悤�ɁA���Y�I�u�W�F�N�g��IDisposable::Dispose���Ăяo�����B
	// �y�X�^�b�N�Z�}���e�B�N�X�Ɋւ���Q�l�����z
	// https://learn.microsoft.com/en-us/cpp/dotnet/cpp-stack-semantics-for-reference-types [�p��]
	// https://learn.microsoft.com/ja-jp/cpp/dotnet/cpp-stack-semantics-for-reference-types [���{��]
	// �s���Ӂt
	// �X�^�b�N�Z�}���e�B�N�X�̐����Ƃ��������A����͊ȒP�Șb�Ƃ��đ��X�ɕЕt���Ă��܂��A���R�s�[�̔����Ɋւ��ďڂ����������Ă���c���̂悤�Ɍ�����B
	// �R�s�[�Ɋւ�������́A���T���v���ŋc�_�̑ΏۂƂ��Ă����O���S�iexception safety�j�ɂ����Ă͂��̍ۏd�v�ł͂Ȃ��B
	// ����āA���̎�����ǂ�Ŗ󂪕�����Ȃ��Ȃ����Ƃ��Ă����ɖ��͂Ȃ��B�i�Ǝv���j
	System::IO::FileStream file(path, System::IO::FileMode::Create, System::IO::FileAccess::Write);
	file.Write(data, 0, data->Length);
	operation();
}
void OperationNone()
{
	//�������Ȃ�
}
void OperationThrowException()
{
	throw gcnew System::Exception(L"��O����������܂����B");
}
int main()
{
	System::Console::Write(
		L"�t�@�C���������ݏ������ɗ�O�����������ꍇ�Ƀn���h�������[�N���邩�ǂ������m���߂܂��B\n"
		L"�n���h�������[�N���Ă���Γ����t�@�C���̏������ݗp�I�[�v���͎��s���܂��B\n"
		L"�n���h�������[�N���Ă��Ȃ���Γ����t�@�C���̏������ݗp�I�[�v���͐������܂��B\n"
	);
	auto data = gcnew array<System::Byte>{ 'A', 'B', 'C' };
	for (const auto& r : 
	{ std::make_tuple(L"\nTest1: ��O�ɂ��n���h���̃��[�N"             , Test1_LeakByException                   , L"Test1Out.txt")
	, std::make_tuple(L"\nTest2: finally��delete���s�����Ƃɂ���O���S", Test2_ExceptionSafeByFinallyDelete      , L"Test2Out.txt")
	, std::make_tuple(L"\nTest3: �X�^�b�N�Z�}���e�B�N�X�ɂ���O���S"   , Test3_ExceptionSafeByUsingStackSemantics, L"Test3Out.txt")
	}) {
		System::Console::WriteLine("\n�e�X�g���n�߂܂��B[Enter] �L�[�������Ă��������B");
		System::Console::ReadKey();
		System::Console::WriteLine(gcnew System::String(std::get<0>(r)));
		for (int i = 0; i < 4; ++i) {
			try {
				System::Console::WriteLine(L"{0} ���", i + 1);
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
pat********����

2025/1/18 23:02

1��

C++/CLI�ŁA�t�@�C���A�N�Z�X���ɔ���������O���L���b�`���A�m���Ƀt�@�C�����N���[�Y����T���v���v���O�������쐬���ĉ������B
���̃v���O�����ł́A��O�̔����ɂ���Ă̓t�@�C�����N���[�Y���ꂸ�ɏI���\��������܂��B
tss1 = gcnew IO::FileStream(ts1, IO::FileMode::Open, IO::FileAccess::Read);
binBuffer = gcnew array<byte>(tss1->Length);

tss1->Read(binBuffer, 0, (int)(tss1->Length));
tss1->Close();

C����֘A�E16�{��
*/
