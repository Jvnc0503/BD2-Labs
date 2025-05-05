import psycopg2
import pandas as pd
import nltk
import json
import re
import warnings
import time

nltk.download("punkt")
nltk.download("punkt_tab")
nltk.download("wordnet")
nltk.download("omw-1.4")
stemmer = nltk.stem.SnowballStemmer("spanish")
lemmatizer = nltk.stem.WordNetLemmatizer()

# Omitir advertencias de pandas sobre SQLAlchemy
warnings.filterwarnings(
    "ignore",
    message="pandas only supports SQLAlchemy connectable",
    category=UserWarning,
)

def connect_db():
    conn = psycopg2.connect(
        dbname="Lab5.1",
        user="postgres",
        password="postgres",
        host="localhost",
        port="5432",
    )
    return conn

def fetch_data():
    conn = connect_db()
    query = "SELECT id, contenido FROM noticias;"
    df = pd.read_sql(query, conn)
    conn.close()
    return df

def fetch_stopwords():
    conn = connect_db()
    query = "SELECT word FROM stopwords;"
    df = pd.read_sql(query, conn)
    conn.close()
    stopword_list = df["word"].tolist()
    return stopword_list

def preprocess(text):
    text = text.lower()
    text = re.sub(r'[^a-záéíóúñü\s]', '', text)
    tokens = nltk.word_tokenize(text, "spanish")
    filtered = [token for token in tokens if token not in stopwords]
    stem = [stemmer.stem(w) for w in filtered]
    return stem

def compute_bow(text):
    processed_text = preprocess(text)
    bow = dict()
    for word in processed_text:
        if word in bow:
            bow[word] += 1
        else:
            bow[word] = 1
    return bow

def update_bow_in_db(dataframe):
    conn = connect_db()
    cursor = conn.cursor()
    for index, row in dataframe.iterrows():
        bow = compute_bow(row["contenido"])
        query = "UPDATE noticias SET bag_of_words = %s WHERE id = %s;"
        cursor.execute(query, (json.dumps(bow), row["id"]))
    conn.commit()
    cursor.close()
    conn.close()

# grammar:
# S -> S A B | keyword
# B -> S | keyword
# A -> OR | AND | AND-NOT

def apply_boolean_query(query, table):
    tokens = query.split()  # list containing query arguments

    # simple sequential parser:
    expects_keyword = True
    final_query = f"SELECT * FROM {table} WHERE bag_of_words ? "

    for w in tokens:
        if expects_keyword:
            stemmed = stemmer.stem(w.lower())
            final_query += "'" + stemmed + "'"
            expects_keyword = False
        else:
            w = w.lower()
            if w == "or":
                final_query += " OR bag_of_words ? "
            elif w == "and":
                final_query += " AND bag_of_words ? "
            elif w == "and-not":
                final_query += " AND NOT bag_of_words ? "
            else:
                print("ERROR: INVALID QUERY, UNRECOGNIZED OPERATOR " + w)
                return pd.DataFrame()
            expects_keyword = True

    if expects_keyword is True:
        print("ERROR: EXPECTED KEYWORD AT END")

    final_query += ";"

    # actual query time
    conn = connect_db()
    df = pd.read_sql(final_query, conn)
    conn.close()

    return df

def test():
    test_queries = [
        "ingeniería OR software AND desarrollo",
        "inteligencia AND artificial AND-NOT humano",
        "ciencia OR tecnología AND-NOT medicina",
        "educación AND aprendizaje OR enseñanza",
        "computción AND matemática AND-NOT física",
        "derecho OR leyes AND justicia",
        "historia OR geografía AND-NOT política",
        "arte OR cultura AND-NOT entretenimiento",
        "musica AND danza OR teatro",
        "salud AND bienestar OR medicina",
    ]

    tables = ["noticias", "noticias600", "noticias300", "noticias150"]
    results = []
    time_totals = {
        "noticias": [],
        "noticias150": [],
        "noticias300": [],
        "noticias600": []
    }
    
    for table in tables:
        for query in test_queries:
            start = time.time()
            df = apply_boolean_query(query, table)
            end = time.time()
            elapsed_ms = (end - start) * 1000  # Convertir a milisegundos
            time_totals[table].append(elapsed_ms)
            results.append(
                {"tabla": table, "query": query, "tiempo_ms": elapsed_ms, "resultados": len(df)}
            )
            print(f"{table} | {query} | {elapsed_ms:.2f} ms | {len(df)} resultados")

    print("\nPromedio de tiempo por tabla:")
    for table in tables:
        times = time_totals[table]
        avg = sum(times) / len(times) if times else 0
        print(f"{table}: {avg:.2f} ms")

    with open("resultados.json", "w", encoding="utf-8") as f:
        json.dump(results, f, ensure_ascii=False, indent=4)

    print("Resultados guardados en resultados.csv")
    

noticias_df = fetch_data()
stopwords = fetch_stopwords()
print(noticias_df)
#update_bow_in_db(noticias_df)
#test()

