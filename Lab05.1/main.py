import psycopg2
import pandas as pd
import nltk
import json

nltk.download("punkt")
nltk.download("punkt_tab")
stemmer = nltk.stem.SnowballStemmer("spanish")

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
    

noticias_df = fetch_data()
stopwords = fetch_stopwords()
update_bow_in_db(noticias_df)