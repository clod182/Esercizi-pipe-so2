public class Filosofi implements Runnable {
	static final int NTHREAD=5;				// numero di filosofi
	final int index;						// indice locale del filosofo
	static Tavola t= new Tavola(NTHREAD);	// monitor statico/condiviso
 
	// il filosofo memorizza il proprio indice
	Filosofi(int i) {
		index = i;
	}
	
	// il thread esegue il codiceFilosofo a meno di interruzioni
	public void run()  {
		try { 
			codiceFilosofo(index);
		} catch (InterruptedException e) {
			System.out.println("Il filosofo "+index+" e' stato interrotto");
		}
	}
 
	// il filosofo pensa e mangia come al solito
	void codiceFilosofo(int index) throws InterruptedException {
		while(true) {
			// PENSA 
			System.out.println("Filosofo " + index +" pensa");
			Thread.sleep(1000);
			
			t.raccogli_sx(index);	// raccoglie la bacchetta sinistra
			// Decommentare per forzare lo stallo
			// Thread.sleep(1000);
			t.raccogli_dx(index);	// raccoglie la bacchetta destra
 
			//MANGIA
			System.out.println("Filosofo " + index +" mangia");
			Thread.sleep(1000);
 
			t.deposita_sx(index);	// deposita la bacchetta sinistra
			t.deposita_dx(index);	// deposita la bacchetta destra
		}
	}
 
	public static void main(String args[]) throws InterruptedException {
		int i;
		Thread t[] = new Thread[NTHREAD];
		
		// crea NTHREAD filosofi e li esegue
		for(i=0;i<NTHREAD;i++) {
			t[i] = new Thread(new Filosofi(i));
			t[i].start();
		}
		// esce lasciando i filosofi al loro destino
	}
}
 
/* monitor Tavola per la gestione delle bacchette */
class Tavola {
	private boolean b[];		// le bacchette
	private final int NFIL;		// il numero di filosofi
 
	// crea le NFIL bacchette e le inizializza a true (presenti)
	// NOTA: non serve sincronizzarlo e' prima della creazione dei filosofi
	Tavola(int i) { 
		int j;
		NFIL=i; 				// memorizza il numero dei filosofi
		b = new boolean[NFIL]; 	// crea le bacchette
		for (j=0;j<NFIL;j++)	// inizializza le bacchette a true
			b[j] = true;
	}
 
	// raccoglie la bacchetta sinistra
	synchronized void raccogli_sx(int i) throws InterruptedException {
		while (!b[i]) // finche' non e' disponibile attende
			wait();
			
		// raccoglie la bacchetta
		b[i]=false;
	}
	// raccoglie la bacchetta destra
	synchronized void raccogli_dx(int i) throws InterruptedException {
		raccogli_sx((i+1)%NFIL); // la dx e' sx del filosofo successivo
	}	
 
	// deposita la bacchetta sinistra e notifica TUTTI i filosofi
	synchronized void deposita_sx(int i) {
		b[i]=true;		// raccoglie la bacchetta
		notifyAll();	// notifica tutti
	}
	// deposita la bacchetta destra e notifica TUTTI i filosofi
	synchronized void deposita_dx(int i) {
		deposita_sx((i+1)%NFIL); // la dx e' sx del filosofo successivo
	}	
}