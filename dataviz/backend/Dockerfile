FROM golang

COPY *.go /go/src/local/dataviz-backend/
WORKDIR /go/src/local/dataviz-backend/
RUN go get

RUN CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -a -tags netgo -ldflags '-w' -o dataviz-backend *.go

FROM alpine
RUN mkdir -p /opt/dataviz-backend
COPY --from=0 /go/src/local/dataviz-backend/dataviz-backend /opt/dataviz-backend/

CMD /opt/dataviz-backend/dataviz-backend
