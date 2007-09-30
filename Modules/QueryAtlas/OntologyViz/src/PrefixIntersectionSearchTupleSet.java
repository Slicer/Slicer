import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.HashSet;
import java.util.Vector;
import prefuse.data.Tuple;
import prefuse.data.search.*;


/**
 * <p>
 * SearchTupleSet implementation supporting word prefix searches over indexed
 * Tuple data fields. This class uses a {@link Trie Trie} data structure
 * to find search results quickly; however, only prefix matches will be
 * identified as valid search matches. Multi-term search queries will result
 * in the union of the results for the individual query terms. That is, Tuples
 * that match any one of the terms will be included in the results.
 * </p>
 * 
 * <p>
 * For more advanced search capabilities, see 
 * {@link KeywordSearchTupleSet} or {@link RegexSearchTupleSet}.
 * </p>
 *
 * @version 1.0
 * @author <a href="http://jheer.org">jeffrey heer</a>
 * @see prefuse.data.query.SearchQueryBinding
 */
public class PrefixIntersectionSearchTupleSet extends SearchTupleSet {
    
    private Trie m_trie;
    private Trie.TrieNode m_curNode;
    private String m_delim = " _\t\n\r";
    private String m_query = "";
    private String m_field = null;
    
    /**
     * Creates a new KeywordSearchFocusSet that is not case sensitive.
     */
    public PrefixIntersectionSearchTupleSet() {
        this(false);
    }
    
    /**
     * Creates a new KeywordSearchFocusSet with the indicated case sensitivity.
     * @param caseSensitive true if the search routines should be case
     * sensitive, false otherwise.
     */
    public PrefixIntersectionSearchTupleSet(boolean caseSensitive) {
        m_trie = new Trie(caseSensitive);
    }
    
    /**
     * Returns the delimiter string used to divide data values and
     * queries into separate words. By default, the value consists
     * of just whitespace characters.
     * @return the delimiter string used. This is passed as an argument to a
     * {@link java.util.StringTokenizer} instance that will tokenize the text.
     * @see java.util.StringTokenizer
     */
    public String getDelimiterString() {
        return m_delim;
    }
    
    /**
     * Sets the delimiter string used to divide data values and
     * queries into separate words. By default, the delimiter consists
     * of just whitespace characters.
     * @param delim the delimiter string to use. This is passed as an argument
     * to a {@link java.util.StringTokenizer} instance that will tokenize the
     * text.
     * @see java.util.StringTokenizer
     */
    public void setDelimiterString(String delim) {
        m_delim = delim;
    }
    
    /**
     * @see prefuse.data.search.SearchTupleSet#getQuery()
     */
    public String getQuery() {
        return m_query;
    }
    
    /**
     * Searches the indexed Tuple fields for matching string prefixes, 
     * adding the Tuple instances for each search match to this TupleSet.
     * The query string is first broken up into separate terms, as determined
     * by the current delimiter string. A search for each term is conducted,
     * and all matching Tuples are included in the results.
     * @param query the query string to search for.
     * @see #setDelimiterString(String)
     */
    public void search(String query) {
        if ( query == null )
            query = "";
        
        if ( query.equals(m_query) )
            return;
        
        Tuple[] rem = clearInternal();    
        m_query = query;

    if (query.startsWith("=")) {
        exactSearch(query.substring(1));
    }
    else {
        StringTokenizer st = new StringTokenizer(m_query, m_delim);
        if ( !st.hasMoreTokens() )
        m_query = "";


        boolean firstTime = true;
        while ( st.hasMoreTokens() ) {
        String tok = st.nextToken();
        prefixSearch(tok, firstTime);
        firstTime = false;
        }
    }
        Tuple[] add = getTupleCount() > 0 ? toArray() : null;
        fireTupleEvent(add, rem);
    }

    private void exactSearch(String query) {
    m_curNode = m_trie.find(query);
    if (m_curNode != null) {
        Iterator iter = trieIterator();
        while (iter.hasNext()) {
        Tuple t = (Tuple)iter.next();
        if (t.getString(m_field).equalsIgnoreCase(query)) {
            addInternal((Tuple)t);
            break;
        }
        }
    }
    }


    /**
     * Issues a prefix search and collects the results
     */
    private void prefixSearch(String query, boolean init) {
    if (init) {
        m_curNode = m_trie.find(query);
        if ( m_curNode != null ) {
        Iterator iter = trieIterator();
        while ( iter.hasNext() )
            addInternal((Tuple)iter.next());
        }
    }
    else {
        m_curNode = m_trie.find(query);
        if ( m_curNode == null ) {
        clearInternal();
        }
        else {
        HashSet<Tuple> hset = new HashSet<Tuple>();
        Iterator iter = trieIterator();
        while ( iter.hasNext() ) {
            hset.add((Tuple)iter.next());
        }
        Iterator titer = tuples();
        Vector<Tuple> toRemove = new Vector<Tuple>();
        while(titer.hasNext()) {
            Tuple t = (Tuple)titer.next();
            if (!hset.contains(t)) {
            toRemove.add(t);
            }
        }
        Iterator riter = toRemove.iterator();
        while (riter.hasNext()) {
            removeInternal((Tuple)riter.next());
        }
        }
    }
    }
    
    /**
     * Indexes the given field of the provided Tuple instance.
     * @see prefuse.data.search.SearchTupleSet#index(prefuse.data.Tuple, java.lang.String)
     */
    public void index(Tuple t, String field) {
    m_field = field;
        String s;
        if ( (s=t.getString(field)) == null ) return;
    addString("*", t);
    addString(s, t);
        StringTokenizer st = new StringTokenizer(s,m_delim);
        while ( st.hasMoreTokens() ) {
            String tok = st.nextToken();
        if (! tok.equals(s)) {
        addString(tok, t);
        }
        }
    }
    
    private void addString(String s, Tuple t) {
        m_trie.addString(s,t);
    }
    
    /**
     * Returns true, as unidexing is supported by this class.
     * @see prefuse.data.search.SearchTupleSet#isUnindexSupported()
     */
    public boolean isUnindexSupported() {
        return true;
    }
    
    /**
     * @see prefuse.data.search.SearchTupleSet#unindex(prefuse.data.Tuple, java.lang.String)
     */
    public void unindex(Tuple t, String field) {
        String s;
        if ( (s=t.getString(field)) == null ) return;
        StringTokenizer st = new StringTokenizer(s,m_delim);
        while ( st.hasMoreTokens() ) {
            String tok = st.nextToken();
            removeString(tok, t);
        }
    }
    
    /**
     * Removes all search hits and clears out the index.
     * @see prefuse.data.tuple.TupleSet#clear()
     */
    public void clear() {
        m_trie = new Trie(m_trie.isCaseSensitive());
        super.clear();
    }
    
    private void removeString(String s, Tuple t) {
        m_trie.removeString(s,t);
    }
    
    private Iterator trieIterator() {
        return m_trie.new TrieIterator(m_curNode);
    }
    
}  // end of class PrefixIntersectionSearchTupleSet

