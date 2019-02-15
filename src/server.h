//
// Created by chawki chouib on 2019-02-15.
//

#ifndef WEB_SERVER_SERVER_H
#define WEB_SERVER_SERVER_H


int findSuccess(char *buf, int rc);

void reponse(int succes, char *rep, int size);

char *decoupe(char *buf, int size);

#endif //WEB_SERVER_SERVER_H
