# �����׻��ץ�����ߥ󥰸��� "revapp"�Ρ�����¤Υ��󥿥ץ꥿����
���η��ˡ§�򱦷���·�����������˴�Ť���ɾ����ά��ɬ�׸ƤӤ��Ѥ��뤳�Ȥǡ��ʷ餵��ޤä��ץ�����ߥ󥰸��졣
�����Ƥ��θ���ǽ񤫤줿�����������ɤ����ɽ�����Ѵ����뤳�Ȥʤ����Τޤޥ����˥��ԡ����Ʋ��¹Ԥ��롢��������ϡ�

## ư��ƥ���
GNU C ����ѥ��餬���åȥ��å׺Ѥߤ�Unix�Ķ��Ǥξ��

	$ make all

�ǥ��󥿥ץ꥿�ΥХ��ʥ�revappi.out����������ޤ���

	$ ./revappi.out example/fizzbuzz.revapp

��FizzBuzz����β������䡹��ɸ����Ϥ˽񤭽Ф���³���ޤ���
ü������Ctrl+C�����Ϥ���ʤɤ��ƶ�����λ���Ʋ�������
�����륹����ץȤˤ�Ȥ���褦�����Υ��󥿥ץ꥿�ϲ�����ʸˡ�˲ä���shebang�Ԥ�ǧ�����ޤ���

	$ ./example/cat.revapp < Makefile

## ʸˡ
revapp��ʸˡ��BNF��ˡ��ɽ���Ȱʲ����ͤˤʤ�ޤ���

	<expr> ::= "="<identifier> | <identifier> | <expr> <expr>

��������`<identifier>`��ʸ������0�ʾ��ʸ����Ȥ��ޤ��� 
����̤��������뤿��˳�̤�Ȥ����Ȥ�����ޤ������å����ʤ���硢���Ƥι�ʸ�ϱ����ͥ��θ�§�˽����ޤ����㤨��"a (=b c)"�Ǥϡ�"=b c"��Ϥ५�å������פ�"a =b c"�Ƚ񤤤Ƥ�Ʊ����̣�ˤʤ�ޤ�����̤ޤǴޤ᤿ʸˡ��

	1. <term> ::= <identifier> | "="<identifier> |"(" <expr> <expr> ")"
	2. <expr> ::= "(" <expr> ")" <expr> | <term>

�Ȥʤ�ޤ���

## "revapp"����ͳ
�����ϡ��֥������פβ��褬�ְ�äƤ����Τ��ˤĤ��ơ�

��������ɽ������׻��ηϡ������׻��ϡ����塼��󥰴����ʺǾ��η׻��ηϤǤ�����Ĥ��Ѵ���§�Ȱ�Ĥδؿ������§����������������Ǥ��ơ����湽¤��ɽ������ǡ����ΰ���ݤޤǡ��ץ�����ߥ󥰤˽�ʬ�ʵ�ǽ���󶡤��Ƥ��ޤ�����äƥ����׻�����ľ��ɽ������С����塼��󥰴����ʺǾ��Υץ�����ߥ󥰸��줬���ޤ��Ϥ��Ǥ���������������ˡ������׻������ߤοƤǤ��륢���󥾡����㡼���ȥ��ƥ������󡦥����롦���꡼�ͤϡ���������ο����ηϤ�ɽ��ˡ��Ȥäƥ����׻���ɽ�����ޤ����������������ޤ줿�Τ��֥������פǤ���

��Ĥΰ���x,y����ؿ�f(x,y)=x+y��ͤ��Ƥߤޤ��礦�����㡼��ã����ä��������Ǥϡ���μ��κ��դ˸��줿�ѿ��ν��֤�Ƨ�����ơ�f=��xy.x+y�פ�ɽ�����ޤ����ޤ����ؿ�Ŭ�Ѥν��֤⡢���ؤ�ɽ���˶ᤤ��f 3 2 = f(3,2)�פȤʤ�褦�˺�������ᡢ��f 3 2�פȡ�(f 3) 2�פ�Ʊ�����ˤʤ�褦��ɽ�����ޤ����Ȥ������Ȥϡ���(��xy.x+y) 3�פ��ͤˡ���������ļ��ؿ��˰�Ĥ����ͤ�Ŭ�Ѥ����硢�˵����ľ����ѿ������������Ԥ�����ˤʤ�ޤ������äơ��֦�xy.x+y�פϡ֦�x.(��y.x+y)�פ�Ʊ�ͤȤʤꡢ�ѿ��ؤ������ϱ����Ȥʤ�ޤ�����̡��ؿ�Ŭ�Ѥȥ�����ݤǤϷ��ν��֤��դˤʤäƤ��ޤ����ޤ����֦�x.g x 3�פȡ�(��x.g x) 3�פ��ͤˡ�������ݤ�«������Ƥ����ѿ��ȡ�������ݤγ�¦�ΰ�������̤��뤿�ᡢ��̤�¿�Ѥ��ʤ���Фʤ�ޤ��󡣤����Ʋ���ꡢ��f=��xy.x+y�פ��ͤˡ��ؿ�f���������Τ������=�פ�ɬ�פȤʤ�ޤ���

�����ͤ˾�Ĺ��ɽ��ˡ����㡼��ã�����Ф�������ʤ��ä��Τϡ����ߤοƤν�̿�Ǥ��������׻�ȯ������ϡ����㡼��ã�ʳ��ο͡��ϥ����׻����Τ�ʤ�����Ǥ��������׻�����ľ��ɽ���ϡ������׻����Τ�ʤ��ͤ˼�äƤϰŹ�ʸ�˲᤮�ޤ��󡣤����������㡼��ã�ΰ����ȯ�����Ｑ�Ȥʤä����������ξ�Ĺ��ɽ��ˡ�˹���ɬ�פϤ���ޤ���

����̾�������褦��revapp�Ǥϡ��ؿ�Ŭ�Ѥν��֤�դˤ��ơ��ؿ�Ŭ�Ѥ������ݤⱦ�������줷�Ƥ��ޤ���

���Τ��ᡢ�����鱦�إ����������ɤθ���̤���ɤ߿ʤ�롢�ʷ����ˡ�Ǥβ��¹Ԥ���ǽ�ˤʤ�ޤ���¿���θ�������ϤΥ��󥿥ץ꥿�����Ǥϡ������������ɤ���Ϥ�����ݹ�ʸ�ڤʤɤ����ɽ�����Ѵ����Ƥ��顢�������ɽ�����оݤ��༡�¹Ԥ�����ˡ������Ū�Ǥ����������ѿ��ʤɤΥ����������٤�ʸ������Ӥ򤹤�ޥ��󥵥������ϲ���ʤ��ʤɡ���®������ͳ�ΰ�ĤǤϤ���Ǥ��礦���������⤦��Ĥ���ͳ��ñ�ˡ������������ɤ��ΤޤޤǤ��༡�¹Ԥ���˾Ū�ˤʤ�ۤɤΡ�ʣ����ʸˡ�Τ��ᡢ�Ȼ�ϵ��äƤ��ޤ��������󥿥ץ꥿�����Ǥϡ������������ɤ򤽤Τޤޥ���˳�Ǽ�������Υ��ꥤ�᡼�����оݤ��༡�¹Ԥ�����ˡ��ΤäƤ��ޤ���

ɽ����ʷ�ˤʤ�ޤ�����3 ��x g x�פ��ͤ˥�����ݤ�«����ʬ�Ȱ�����ʬ���˵����ʬ�Ǥ��졢��̤λ��Ѥ�����Ǥ��ޤ����ޤ����ѿ��򼨤���������ȡ������ѿ���Ŭ�Ѥ������������ܤ���������Ǥ�����������������=�פ��ͤ��Ѥ��뤳�Ȥ�����ޤ���revapp�Ǥϡ���������ǤϤʤ�����ǥ�����ݤ�ɽ�����ޤ���