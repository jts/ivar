#include<string>
#include<vector>
#include<algorithm>
#include<iostream>

#include "allele_functions.h"

void print_allele_depths(std::vector<allele> ad){
  std::cout << "AD Size: " << ad.size() << " ";
  for(std::vector<allele>::iterator it = ad.begin(); it != ad.end(); ++it) {
    std::cout << it->nuc << " ";
    std::cout << it->depth << " ";
    std::cout << it->reverse << " ";
    std::cout << (uint16_t) it->mean_qual << " ";
  }
  std::cout << std::endl;
}

int check_allele_exists(std::string n, std::vector<allele> ad){
  for(std::vector<allele>::iterator it = ad.begin(); it != ad.end(); ++it) {
    if(it->nuc.compare(n) == 0){
      return it - ad.begin();
    }
  }
  return -1;
}

int find_ref_in_allele(std::vector<allele> ad, char ref){
  std::string ref_s(1, ref);
  std::vector<allele>::iterator it = ad.begin();
  while(it < ad.end()){
    if(it->nuc.compare(ref_s) == 0)
      return (it - ad.begin());
    it++;
  }
  return -1;
}

std::vector<allele> update_allele_depth(char ref,std::string bases, std::string qualities, uint8_t min_qual){
  std::vector<allele> ad;
  std::string indel;
  int i = 0, n =0, j = 0, q_ind = 0;
  uint8_t q;
  while (i < bases.length()){
    if(bases[i] == '^'){
      i += 2;			// Skip mapping quality as well (i+1) - 33
      continue;
    }
    if(bases[i] == '$'){
      i++;
      continue;
    }
    q = qualities[q_ind] - 33;
    if(q < min_qual){
      i++;
      q_ind++;
      continue;
    }
    std::string b;
    allele tmp;
    bool forward= true;
    switch(bases[i]){
    case '.':
      b = ref;
      break;
    case ',':
      b = ref;
      forward = false;
      break;
    case '*':
      b = bases[i];
      break;
    case '+': case '-':
      j = i+1;
      while(isdigit(bases[j])){
	j++;
      }
      j = j - (i+1);
      n = stoi(bases.substr(i+1, j));
      indel = bases.substr(i+1+j, n);
      transform(indel.begin(), indel.end(), indel.begin(),::toupper);
      b = bases[i] + indel;	// + for Insertion and - for Deletion
      i += n + 1;
      if(indel[0]>=97 && indel[0] <= 122)
	forward=false;
      break;
    default:
      int asc_val = bases[i];
      if(asc_val >= 65 && asc_val <= 90){
	b = bases[i];
      } else if(bases[i]>=97 && bases[i]<=122){
	b = bases[i] - ('a' - 'A');
	forward = false;
      }
    }
    int ind = check_allele_exists(b, ad);
    if(q >= min_qual){
      if (ind==-1){
	tmp.nuc = b;
	tmp.depth = 1;
	tmp.mean_qual = q;
	if(!forward)
	  tmp.reverse = 1;
	else
	  tmp.reverse = 0;
	ad.push_back(tmp);
      } else {
	ad.at(ind).mean_qual = ((ad.at(ind).mean_qual * ad.at(ind).depth) + q)/(ad.at(ind).depth + 1);
	ad.at(ind).depth += 1;
	if(!forward)
	  ad.at(ind).reverse += 1;
      }
    }
    i++;
    if(b[0] !='+' && b[0]!='-')
      q_ind++;
  }
  std::sort(ad.begin(), ad.end());
  return ad;
}

int get_index(char a){
  switch(a){
  case'Y':
    a = 0;
    break;
  case'R':
    a = 1;
    break;
  case'W':
    a = 2;
    break;
  case'S':
    a = 3;
    break;
  case'K':
    a = 4;
    break;
  case'M':
    a = 5;
    break;
  case'A':
    a = 6;
    break;
  case'T':
    a = 7;
    break;
  case'G':
    a = 8;
    break;
  case'C':
    a = 9;
    break;
  default:
    a = -1;
  }
  return (int) a;
}

// Modified gt2iupac from bcftools.h. Expanded iupac matrix to include all ambigious nucleotides(added Y, R, Q, S, K, M)  - https://github.com/samtools/bcftools/blob/b0376dff1ed70603c9490802f37883b9009215d2/bcftools.h#L48
/*
Expanded IUPAC matrix:

Y N H B B H H Y B Y
N R D V D V R D R V
H D W N D H W W D H
B V N S B V V B S S
B D D B K N D K K B
H V H V N M M H V M
H R W V D M A W R M
Y D W B K H W T K Y
B R D S K V R K G S
Y V H S B M M Y S C

 */
char gt2iupac(char a, char b){
  static const char iupac[10][10] = {
    {'Y', 'N', 'H', 'B', 'B', 'H', 'H', 'Y', 'B', 'Y'},
    {'N', 'R', 'D', 'V', 'D', 'V', 'R', 'D', 'R', 'V'},
    {'H', 'D', 'W', 'N', 'D', 'H', 'W', 'W', 'D', 'H'},
    {'B', 'V', 'N', 'S', 'B', 'V', 'V', 'B', 'S', 'S'},
    {'B', 'D', 'D', 'B', 'K', 'N', 'D', 'K', 'K', 'B'},
    {'H', 'V', 'H', 'V', 'N', 'M', 'M', 'H', 'V', 'M'},
    {'H', 'R', 'W', 'V', 'D', 'M', 'A', 'W', 'R', 'M'},
    {'Y', 'D', 'W', 'B', 'K', 'H', 'W', 'T', 'K', 'Y'},
    {'B', 'R', 'D', 'S', 'K', 'V', 'R', 'K', 'G', 'S'},
    {'Y', 'V', 'H', 'S', 'B', 'M', 'M', 'Y', 'S', 'C'}
  };
  if ( a>='a' ) a -= 'a' - 'A';
  if ( b>='a' ) b -= 'a' - 'A';
  int _a, _b;
  _a = get_index(a);
  _b = get_index(b);
  if(a == -1 || b == -1)
    return 'N';
  return iupac[_a][_b];
}
