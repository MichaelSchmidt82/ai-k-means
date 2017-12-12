/*
     ██████╗ ██╗      █████╗ ██╗  ██╗
    ██╔═══██╗██║     ██╔══██╗██║  ██║
    ██║   ██║██║     ███████║███████║
    ██║   ██║██║     ██╔══██║╚════██║
    ╚██████╔╝███████╗██║  ██║     ██║
     ╚═════╝ ╚══════╝╚═╝  ╚═╝     ╚═╝

          ~=[ The Final Lab ]=~

Author:         Michael Schmidt;
Class:          CSCI 4350 - Intro to Artifical Intelligence
Instructor:     Dr. Joshua Phillips
Due:            5 Dec 2017

Description:    Unsupervised learning by K-means clustering.

Notes/issues:   None Known
*/

#include "globals.h"

/* Prototypes */
inline double term (double x1, double x2) { return pow(x2 - x1, 2); }

DataMatrix init(char * train_name,
    char * test_name,
    DataMatrix & test_out,
    DataMatrix & centroids,
    DataMatrix & distances,
    const size_t K,
    const size_t N_VALS);

bool recenter(
    const DataMatrix & data,
    DataMatrix & centroid,
    const Affiliation & affiliations,
    const size_t K,
    const size_t N_VALS);

void cluster (const DataMatrix & data,
    const DataMatrix & centroids,
    Affiliation & affiliations,
    DataMatrix & distances,
    const size_t K,
    const size_t N_VALS);

void class_check (const DataMatrix & train_data,
    const DataMatrix test_data,
    const Affiliation & train_affils,
    const DataMatrix & centroids,
    const size_t K,
    const size_t N_VALS);

void normailize (DataMatrix & data,
    const size_t N_VALS,
    bool train);

void manhattan_dist (const DataMatrix & data,
    const DataMatrix & centroids,
    Affiliation & affiliations,
    DataMatrix & distances,
    const size_t K,
    const size_t N_VALS);

template<typename T>
inline void quick_swap(T & x, T & y);

/* int main() */
int main (int argc, char * argv[]) {
    assert(argc == 6 && "5 CLA's must be provided.");
    srand(atoi(argv[1]));

    /* Constants */
    const size_t K = atoi(argv[2]);
    const size_t N_VALS = atoi(argv[3]);

    /* Variables */
    DataMatrix centroids(K);
    DataMatrix distances;
    DataMatrix test;
    DataMatrix data = init(argv[4], argv[5], test, centroids, distances, K, N_VALS);
    Affiliation affiliations(data.size());

    /* Algorithm */
    do {
         cluster(data, centroids, affiliations, distances, K, N_VALS);
    } while(!recenter(data, centroids, affiliations, K, N_VALS));

    /* Validate */
    class_check (data, test, affiliations, centroids, K, N_VALS);
    return 0;
}

/* Functions */
void cluster (const DataMatrix & data,
    const DataMatrix & centroids,
    Affiliation & affiliations,
    DataMatrix & distances,
    const size_t K,
    const size_t N_VALS) {

    double sum = 0;

    /* Calculate distances for datum d to centroid k - affiliate to shortest */
    for (size_t d = 0; d < data.size(); d++) {
        double min = numeric_limits<double>::max();
        size_t min_idx = 0;
        for (size_t k = 0; k < K; k++) {

            const Dataset& c = centroids[k];
            for (size_t v = 0; v < N_VALS; v++)
                sum += term(data[d][v], c[v]);

            distances[d][k] = sqrt(sum);

            if (distances[d][k] < min) {
                min = distances[d][k];
                min_idx = k;
            }
            sum = 0;
        }
        affiliations[d] = min_idx;
    }
}

void class_check (const DataMatrix & train_data,
    const DataMatrix test_data,
    const Affiliation & train_affils,
    const DataMatrix & centroids,
    const size_t K,
    const size_t N_VALS) {

    size_t n_classes = 0;
    size_t max = 0;
    size_t correct = 0;

    DataMatrix tallies(K);
    DataMatrix distances(test_data.size());
    Affiliation classifications(K);
    Affiliation test_affils(test_data.size());

    for (Dataset & ds : distances)
        ds = Dataset(K, 0);

    /* Count classifier labels, increment to include 0 */
    for (size_t d = 0; d < train_data.size(); d++)
            if (train_data[d][N_VALS] > n_classes)
                n_classes = int(train_data[d][N_VALS]);
    n_classes++;

    /* Tally up each datum d's affiliation to some cluster */
    for (Dataset & d : tallies)
        d.resize(n_classes, 0);

    for (size_t d = 0; d < train_data.size(); d++)
        tallies[train_affils[d]][train_data[d][N_VALS]]++;

    /* Classify cluster K by most common class label c. Smallest c if tied */
    for (size_t k = 0; k < K; k++) {
        for (size_t c = 0; c < n_classes; c++) {
            if (tallies[k][c] >= max) {
                max = tallies[k][c];
                classifications[k] = c;
            }
        }
        max = 0;
    }

    cluster(test_data, centroids, test_affils, distances, K, N_VALS);

    /* Count how many are correct */
    for (size_t t = 0; t < test_data.size(); t++)
        if (classifications[test_affils[t]] == test_data[t][N_VALS])
            correct++;

    cout << correct << endl;
}

bool recenter (const DataMatrix & data,
    DataMatrix & centroids,
    const Affiliation & affiliations,
    const size_t K,
    const size_t N_VALS) {

    size_t idx = 0;
    size_t n = 0;

    DatasetPtr centroid = nullptr;
    DataMatrix new_centroids(K, Dataset(N_VALS));

    /* Recenter each centroid k in the cluster */
    for (size_t k = 0; k < K; k++) {
        centroid = &new_centroids[k];

        /* Sum each datum d that belong to cluster k */
        for (const size_t aff : affiliations) {
            if (aff == k) {
                for (size_t d = 0; d < N_VALS; d++)
                    (*centroid)[d] += data[idx][d];
                n++;
            }
            idx++;
        }

        /* Calculate new centroids by taking average of n data points */
        for (double & d : *centroid)
            d /= n;

        idx = 0;
        n = 0;
    }

    /* Check for convergence. */
    for (size_t k = 0; k < K; k++)
        for (size_t d = 0; d < centroids[k].size(); d++)
            if (abs(centroids[k][d] - new_centroids[k][d]) > numeric_limits<double>::min()) {
                centroids = move(new_centroids);
                return false;
            }

    return true;
}

DataMatrix init (char* train,
    char* test,
    DataMatrix & test_data,
    DataMatrix & centroids,
    DataMatrix & distances,
    const size_t K,
    const size_t N_VALS) {

    ifstream train_file(train);
    ifstream test_file(test);

    Dataset dataset;
    DataMatrix train_data;
    vector<size_t> staring_pts;
    string line;

    double value;
    size_t train_data_size;
    size_t index = 0;

    assert(test_file.is_open() && train_file.is_open() &&
        "The training and/or testing files failed to open");

    /* Parse training data from file */
    getline(train_file, line);
    while(!train_file.eof()){
    	stringstream digest(line);

        while (digest >> value)
            dataset.push_back(value);

        getline(train_file, line);
        train_data.push_back(dataset);
        dataset.clear();
    }

    train_data_size = train_data.size();
    staring_pts.resize(train_data_size);

    /* Parse testing data from file */
    getline(test_file, line);
    while(!test_file.eof()){
    	stringstream digest(line);

        while (digest >> value)
            dataset.push_back(value);

        getline(test_file, line);
        test_data.push_back(dataset);
        dataset.clear();
    }

    /* Normailize values. Training then testing with training parameters */
    normailize(train_data, N_VALS, true);
    normailize(test_data, N_VALS, false);

    /* Create vector of sequential size_t that will act as indices */
    for (size_t & i : staring_pts) {
        i = index;
        index++;
    }

    /* Shuffle those indices */
    for (size_t p = 0; p < train_data_size; p++)
        quick_swap(staring_pts[p], staring_pts[rand() % train_data_size]);

    /* Use the random indices to select point p for cluster centroid k */
    for (size_t k = 0; k < K; k++)
        centroids[k] = train_data[staring_pts[k]];

    /* Initialize distances to numeric_limits<double>::max() */
    distances.resize(train_data_size);
    for (Dataset & ds : distances)
        ds = Dataset(K, numeric_limits<double>::max());

    /* Tidy up */
	test_file.close();
	train_file.close();
    return train_data;
}

void normailize (DataMatrix & data,
    const size_t N_VALS,
    bool train) {

    /* Keep mins/maxs static to normailize test data with trainging values */
    static Dataset mins(N_VALS);
    static Dataset maxs(N_VALS);

    if (train) {
        for (size_t a = 0; a < N_VALS; a++) {
            double min = numeric_limits<double>::max();
            double max = numeric_limits<double>::max() * -1;
            double denom;
            for (size_t d = 0; d < data.size(); d++) {
                if (data[d][a] < min)
                    min = data[d][a];

                if (data[d][a] > max)
                    max = data[d][a];
            }
            mins[a] = min;
            maxs[a] = max;
            denom = max - min;

            for (size_t d = 0; d < data.size(); d++)
                data[d][a] = (data[d][a] - min) / denom;
        }
    } else {
        for (size_t a = 0; a < N_VALS; a++) {
            double denom = maxs[a] - mins[a];
            for (size_t d = 0; d < data.size(); d++)
                data[d][a] = (data[d][a] - mins[a]) / denom;
        }
    }
}

/* WIP */
// void geometric_mean (DataMatrix * centroid, const size_t K, const size_t N_VALS) {
//     DataMatrix new_centroids(K, Dataset(N_VALS, 1));
//
//     /* Recenter each centroid k in the cluster */
//     for (size_t k = 0; k < K; k++) {
//         centroid = &new_centroids[k];
//
//         /* Sum each datum d that belong to cluster k */
//         for (const size_t aff : affiliations) {
//             if (aff == k) {
//                 for (size_t d = 0; d < N_VALS; d++)
//                     (*centroid)[d] *= data[idx][d];
//                 n++;
//             }
//             idx++;
//         }
//
//         /* Calculate new centroids by taking average of n data points */
//         for (double & d : *centroid)
//             d = pow(d, 1.0 / n);
// }

/* Experimental - Often bad */
void manhattan_dist (const DataMatrix & data,
    const DataMatrix & centroids,
    Affiliation & affiliations,
    DataMatrix & distances,
    const size_t K,
    const size_t N_VALS) {

    double sum = 0;

    for (size_t d = 0; d < data.size(); d++) {
        double min = numeric_limits<double>::max();
        size_t min_idx = 0;
        for (size_t k = 0; k < K; k++) {

            const Dataset& c = centroids[k];
            for (size_t a = 0; a < N_VALS; a++)
                sum += abs(data[d][a] - c[a]);

            distances[d][k] = sum;
            if (distances[d][k] < min) {
                min = distances[d][k];
                min_idx = k;
            }
            sum = 0;
        }
        affiliations[d] = min_idx;
    }
}

template<typename T>
inline void quick_swap(T & x, T & y) {
    if (x != y) {
        x ^= y;
        y ^= x;
        x ^= y;
    }
}