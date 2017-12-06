// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "infer.h"

using namespace ISLE;

// Run ./ISLEInfer
// Output: list of <doc_id> <topic id> <wt>  (small wt entries will be dropped)
int main(int argv, char**argc)
{
    if (argv != 11) {
        std::cout << "Incorrect usage of ISLEInfer. Use: \n"
            << "inferFromFile <sparse_model_file> <infer_file> <output_dir> "
            << "<num_topics> <vocab_size> <num_docs_in_infer_file> "
            << "<nnzs_in_infer_file> <nnzs_in_sparse_model_file>" 
            << "<iters>[0 for default]  "
            << "Lifschitz_constant_guess>[0 for default]" << std::endl;
        exit(-1);
    }
    const std::string sparse_model_file = std::string(argc[1]);
    const std::string infer_file = std::string(argc[2]);
    const std::string output_dir = std::string(argc[3]);

    const docsSz_t num_topics = atol(argc[4]);
    const vocabSz_t vocab_size = atol(argc[5]);
    const docsSz_t num_docs = atol(argc[6]);
    const offset_t max_entries = atol(argc[7]);
    const offset_t M_hat_catch_sparse_entries = atol(argc[8]);

    int iters = atol(argc[9]);
    FPTYPE Lfguess = atof(argc[10]);

    if (iters == 0) iters = 15;
    if (Lfguess == 0.0) Lfguess = 10.0;  

    //std::cout << "Loading sparse model file: " << fileUnderDirNameString(output_dir, sparse_model_file) << std::endl;
    //auto sparse_model = new SparseMatrix<FPTYPE>(vocab_size, num_topics);
    //load_sparse_model_from_file(sparse_model, 
    //    fileUnderDirNameString(output_dir, sparse_model_file), M_hat_catch_sparse_entries);
    

    /*std::cout << "Loading model file: " << fileUnderDirNameString(output_dir, model_file) << std::endl;
    auto model = new DenseMatrix<FPTYPE>(vocab_size, num_topics);
    FPTYPE *model_by_word = new FPTYPE[vocab_size * num_topics];
    load_model_from_file(model, fileUnderDirNameString(output_dir, model_file));
    create_model_by_word(model_by_word, model);
    delete model;*/

    std::cout << "Loading sparse model file: " << sparse_model_file << std::endl;
    FPTYPE *model_by_word = new FPTYPE[vocab_size * num_topics];
    memset(model_by_word, 0, sizeof(FPTYPE)*vocab_size*num_topics);
    load_model_from_sparse_file(model_by_word, num_topics, vocab_size, sparse_model_file);


    std::cout << "Loading data from inference file: " << infer_file << std::endl;
    std::vector<DocWordEntry<count_t> > entries;
    DocWordEntriesReader reader(entries);
    reader.read_from_file(infer_file, max_entries);
    auto infer_data = new SparseMatrix<FPTYPE>(vocab_size, num_docs);
    std::sort(			// Sort by doc first, and word second.
        entries.begin(), entries.end(),
        [](const auto& l, const auto& r)
    {return (l.doc < r.doc) || (l.doc == r.doc && l.word < r.word); });
    entries.erase(			// Remove duplicates
        std::unique(entries.begin(), entries.end(),
            [](const auto& l, const auto& r) {return l.doc == r.doc && l.word == r.word; }),
        entries.end());
    infer_data->populate_CSC(entries);
    infer_data->normalize_docs(true, true);

    docsSz_t doc_block = 1000000;
    for (docsSz_t block = 0; block < divide_round_up(num_docs, doc_block); ++block) {

        std::cout << "Creating inference engine" << std::endl;
        ISLEInfer infer(model_by_word, infer_data, num_topics, vocab_size, num_docs);
        MMappedOutput out(fileUnderDirNameString(output_dir,
            std::string("inferred_weights_iters_") + std::to_string(iters)
            + std::string("_Lf_") + std::to_string(Lfguess)) 
            + std::string("_block_") + std::to_string(block));
        FPTYPE* wts = new FPTYPE[num_topics];
        auto llhs = new FPTYPE[doc_block];
        docsSz_t nconverged = 0;
        for (int doc = block*doc_block; doc < (block + 1)*doc_block && doc < num_docs; ++doc) {
            if (doc % 10000 == 9999)
                std::cout << "#docs inferred: " << doc + 1 << std::endl;

            llhs[doc] = infer.infer_doc_in_file(doc, wts, iters, Lfguess);
            if (llhs[doc] != 0.0)
                nconverged++;
            else std::cout << "Doc: " << doc << "failed to converge" << std::endl;
            for (docsSz_t topic = 0; topic < num_topics; ++topic)
                out.concat_float(llhs[doc] == 0.0 ? 1.0 / (FPTYPE)num_topics : wts[topic], '\t', 1, 8);
            out.add_endline();
        }
        out.flush_and_close();

        std::cout << "Number of docs for which inference converged: " << nconverged
            << " (of " << num_docs << ")" << std::endl;
        std::cout << "Avg LLH for converged docs: "
            << std::accumulate(llhs, llhs + num_docs, 0.0) / nconverged << std::endl;

        delete[] llhs;
        delete[] wts;
    }
    
    delete infer_data;
    delete[] model_by_word;

    return 0;
}
