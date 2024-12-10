import java.util.List;
import java.util.ArrayList;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

/**
 * テキストファイルの文字が格子状に並んでいると仮定し、上下および左右で、
 * 異なる文字の間にだけ罫線を引く。スペース(文字がない場合も含む)と普通の文字の間には、
 * 太い罫線を引く。
 */
class Frame1 {
  static final char[] frameChar={
    '　','〓','〓','〓','─','─','〓','─','━',
    '〓','┘','┘','└','┴','┴','└','┴','┷',
    '〓','┘','┛','└','┸','┸','┗','┸','┻',
    '〓','┐','┐','┌','┬','┬','┌','┬','┯',
    '│','┤','┥','├','┼','┼','┝','┼','┿',
    '│','┤','┥','├','┼','┼','┝','┼','┿',
    '〓','┐','┓','┌','┰','┰','┏','┰','┳',
    '│','┤','┥','├','┼','┼','┝','┼','┿',
    '┃','┨','┫','┠','╂','╂','┣','╂','╋'
  };
  static int cmp(int c0,int c1) {
    if(c0<0 || c0==' ' || c0=='　') return c1<0 || c1==' ' || c1=='　' ? 0 : 2;
    if(c1<0 || c1==' ' || c1=='　') return 2;
    return c0==c1 ? 0 : 1;
  }
  static int get(String s,int i) {
    return i>=0 && i<s.length() ? s.charAt(i) : -1;
  }
  static void display(List sl) {
    String s0="";
    for(int i=0,n=sl.size();i<=n;i++) {
      String s=i<n ? (String)sl.get(i) : "";
      int m=Math.max(s0.length(),s.length());
      StringBuffer sb=new StringBuffer();
      for(int j=0;j<=m;j++) {
        int k=cmp(get(s0,j-1),get(s,j-1))+cmp(get(s0,j),get(s,j))*3+
              cmp(get(s0,j-1),get(s0,j))*9+cmp(get(s,j-1),get(s,j))*27;
        sb.append(frameChar[k]);
      }
      System.out.println(sb);
      s0=s;
    }
  }
  public static void main(String[] args) {
    if(args.length!=1) { System.out.println("使い方: java Frame file.txt"); System.exit(2); }
    List sl=new ArrayList();
    try {
      BufferedReader br=new BufferedReader(new FileReader(args[0]));
      for(;;) {
        String s=br.readLine();
        if(s==null) break;
        sl.add(s);
      }
      br.close();
    } catch(IOException e) {
      System.out.println(e);
    }
    display(sl);
  }
}
