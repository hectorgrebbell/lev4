/**
 * This is not my code. Its a decompiled version of the .class file provided
 * (I'd assume written by Ron Poet - University of Glasgow).
 * 
 * With knowledge of key and p being 16 bit it boils down to -
 * return ((key*16807) % 65536) ^ c;
 * Which makes the reverse
 * return ((22039 * rKey) % 65536) ^ c;
 */

public class Coder
{
	private static int seed;
	private static final int a = 16807;
	private static final int m = 2147483647;
	private static final int q = 127773;
	private static final int r = 2836;

	public static int encrypt(int key, int p)
	{
		return code(key, p);
	}

	public static int decrypt(int key, int c)
	{
		return code(key, c);
	}

	public static int code(int key, int pc)
	{
		seed = key;
		return dice(65536) ^ pc;
	}
	public static int dice(int n)
	{
		int hi = seed / q;
		int lo = seed % q;
		int t = a * lo - r * hi;
		seed = t > 0 ? t : t + m;

		return seed % n;
	}
}
