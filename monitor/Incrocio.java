/*
 * Descrizione generale: utilizziamo 2 contatori
 * 
 * - pedoni: numero di pedoni in attesa o attraversamento
 * - auto: numero di auto in attraversamento
 * 
 * Il contatore pedoni viene incrementato all'inizio di arrivaPedone() in modo da
 * conteggiare sia i pedoni in attesa che quelli in attraversamento. Il contatore
 * auto invece viene incrementato dopo che l'auto ha atteso eventuali pedoni,
 * alla fine del metodo arrivaAuto(), in modo da conteggiare solo le auto in
 * attraversamento (e non quelle in attesa). In questo modo è sufficiente che le
 * auto attendano finché pedoni>0 e che i pedoni attendano finché auto>0.
 * L'ultimo thread auto (o pedone) che esce dall'incrocio esegue una notifyAll
 * per risvegliare eventuali pedoni (o auto) in attesa di poter attraversare
 * l'incrocio.
 * 
 * Dentro il metodo muovi bisogna attendere finché strada[x+dx][y+dy] != ' ', per
 * evitare che auto e pedoni si sovrappongano (l'accesso alla strada deve
 * avvenire in mutua esclusione). Quando viene liberata una cella (metodo libera)
 * si deve fare una notifyAll per sbloccare eventuali pedoni o auto in attesa
 * dentro il metodo muovi.
 */
 
public class Incrocio {
    int DIM;
    char[][] strada;
    int pedoni = 0; // numero di pedoni in attesa o attraversamento
    int auto = 0;   // numero di auto in attraversamento
 
    public Incrocio(int DIM) {
        int i,j;
        this.DIM = DIM;
        this.strada = new char[DIM][DIM];
        for (i=0;i<DIM;i++)
            for (j=0;j<DIM;j++)
                strada[i][j] = ' ';
    }
 
    public synchronized void muovi(char c, int x, int y, int dx, int dy) throws InterruptedException{
        // attende finché la cella in cui deve muoversi è occupata
        while (strada[x+dx][y+dy] != ' ')
            wait();
        strada[x+dx][y+dy]= c;
        libera(x,y);
    }
 
    public synchronized void libera(int x, int y) {
        strada[x][y] = ' ';
        // notifica eventuali auto o pedoni in attesa di muoversi (metodo muovi)
        notifyAll(); 
 
    }
    public char strada(int x, int y) {
        return strada[x][y];
    }
 
 
    public synchronized void arrivaAuto() throws InterruptedException {
        // finché ci sono pedoni in attesa o attraversamento attende
        // (precedenza ai pedoni)
        while(pedoni>0)
            wait();
        auto += 1; // un'auto in più in attraversamento
 
    }
 
    public synchronized void attraversatoAuto() {
        auto -= 1; // un'auto in meno in attraversamento
        if (auto == 0)
            // se è l'ultima auto nell'incrocio notifica eventuali pedoni in attesa
            notifyAll(); 
    }
    public synchronized void arrivaPedone() throws InterruptedException {
        pedoni += 1; // un pedone in più in attesa o attraversamento
        // finché ci sono auto in attraversamento attende (non attende auto in
        // attesa, perché i pedoni hanno la precedenza)
        while(auto > 0)
            wait(); 
    }
 
    public synchronized void attraversatoPedone() {
        pedoni -= 1; // un pedone in meno in attraversamento o attesa
        if (pedoni == 0)
            // se è l'ultimo pedone nell'incrocio notifica eventuali auto in attesa
            notifyAll(); 
    }
}